/* Extension regex 
 * ---------------
 *
 * Authors: Stephan Bosch
 * Specification: draft-murchison-sieve-regex-07
 * Implementation: full, but suboptimal
 * Status: experimental, largely untested
 *
 * FIXME: Regular expressions are compiled during compilation and 
 * again during interpretation. This is suboptimal and should be 
 * changed. This requires dumping the compiled regex to the binary. 
 * Most likely, this will only be possible when we implement regular
 * expressions ourselves. 
 * 
 */

#include "lib.h"
#include "mempool.h"
#include "buffer.h"

#include "sieve-common.h"

#include "sieve-code.h"
#include "sieve-extensions.h"
#include "sieve-commands.h"

#include "sieve-comparators.h"
#include "sieve-match-types.h"

#include "sieve-validator.h"
#include "sieve-generator.h"
#include "sieve-interpreter.h"

#include <sys/types.h>
#include <ctype.h>
#include <regex.h>

/* Forward declarations */

static bool ext_regex_load(int ext_id);
static bool ext_regex_validator_load(struct sieve_validator *validator);
static bool ext_regex_interpreter_load
	(struct sieve_interpreter *interpreter);

void *mtch_regex_match_init
	(const struct sieve_match_type *mtch, const struct sieve_comparator *cmp,
		const char *key, size_t key_size);
static bool mtch_regex_match
	(const struct sieve_match_type *mtch ATTR_UNUSED, 
		const struct sieve_comparator *cmp, const char *val, size_t val_size, 
		const char *key, size_t key_size, void *key_context);
void mtch_regex_match_deinit
	(const struct sieve_match_type *mtch, void *key_context);

/* Extension definitions */

static int ext_my_id;

const struct sieve_extension regex_extension = { 
	"regex", 
	ext_regex_load,
	ext_regex_validator_load,
	NULL, 
	ext_regex_interpreter_load,  
	NULL, 
	NULL
};

static bool ext_regex_load(int ext_id)
{
	ext_my_id = ext_id;

	return TRUE;
}

/* Extension access structures */

extern const struct sieve_match_type_extension regex_match_extension;

bool mtch_regex_validate_context
(struct sieve_validator *validator, struct sieve_ast_argument *arg,
    struct sieve_match_type_context *ctx, struct sieve_ast_argument *key_arg);

const struct sieve_match_type regex_match_type = {
	"regex",
	SIEVE_MATCH_TYPE_CUSTOM,
	&regex_match_extension,
	0,
	NULL,
	mtch_regex_validate_context,
	mtch_regex_match_init,
	mtch_regex_match,
	mtch_regex_match_deinit,
};

const struct sieve_match_type_extension regex_match_extension = { 
	&regex_extension,
	&regex_match_type, 
	NULL
};

/* Validation */

/* Wrapper around the regerror function for easy access */
static const char *_regexp_error(regex_t *regexp, int errorcode)
{
	size_t errsize = regerror(errorcode, regexp, NULL, 0); 

	if ( errsize > 0 ) {
		char *errbuf;

		buffer_t *error_buf = 
			buffer_create_dynamic(pool_datastack_create(), errsize);
		errbuf = buffer_get_space_unsafe(error_buf, 0, errsize);

		errsize = regerror(errorcode, regexp, errbuf, errsize);
	 
		/* We don't want the error to start with a capital letter */
		errbuf[0] = tolower(errbuf[0]);

		buffer_append_space_unsafe(error_buf, errsize);

		return str_c(error_buf);
	}

	return "";
}

static bool mtch_regex_validate_regexp
(struct sieve_validator *validator, struct sieve_match_type_context *ctx,
	struct sieve_ast_argument *key, int cflags) 
{
	int ret;
	regex_t regexp;

	if ( (ret=regcomp(&regexp, sieve_ast_argument_strc(key), cflags)) != 0 ) {
		sieve_command_validate_error(validator, ctx->command_ctx,
			"invalid regular expression for regex match: %s", 
			_regexp_error(&regexp, ret));

		regfree(&regexp);	
		return FALSE;
	}

	regfree(&regexp);
	return TRUE;
}
	
bool mtch_regex_validate_context
(struct sieve_validator *validator, struct sieve_ast_argument *arg,
	struct sieve_match_type_context *ctx, struct sieve_ast_argument *key_arg)
{
	int cflags;
	struct sieve_ast_argument *carg = 
		sieve_command_first_argument(ctx->command_ctx);
	
	cflags =  REG_EXTENDED | REG_NOSUB;
	while ( carg != NULL ) {
		if ( carg != arg && carg->argument == &comparator_tag ) {
			if ( sieve_comparator_tag_is(carg, &i_ascii_casemap_comparator) )
				cflags =  REG_EXTENDED | REG_NOSUB | REG_ICASE;
			else if ( sieve_comparator_tag_is(carg, &i_octet_comparator) )
				cflags =  REG_EXTENDED | REG_NOSUB ;
			else {
				sieve_command_validate_error(validator, ctx->command_ctx, 
					"regex match type only supports "
					"i;octet and i;ascii-casemap comparators" );
				return FALSE;	
			}

			return TRUE;
		}
	
		carg = sieve_ast_argument_next(carg);
	}

	/* Validate regular expression(s) */
	if ( sieve_ast_argument_type(key_arg) == SAAT_STRING ) {
		/* Single string */	
		if ( !mtch_regex_validate_regexp(validator, ctx, key_arg, cflags) )
			return FALSE;

	} else if ( sieve_ast_argument_type(arg) == SAAT_STRING_LIST ) {
		/* String list */
		struct sieve_ast_argument *stritem = sieve_ast_strlist_first(arg);

		while ( stritem != NULL ) {
			if ( !mtch_regex_validate_regexp(validator, ctx, stritem, cflags) )
				return FALSE;

			stritem = sieve_ast_strlist_next(stritem);
		}
	} else {
		/* ??? */ 
		return FALSE;
	} 

	return TRUE;
}

/* Actual extension implementation */

void *mtch_regex_match_init
(const struct sieve_match_type *mtch ATTR_UNUSED, 
	const struct sieve_comparator *cmp, const char *key, 
	size_t key_size ATTR_UNUSED)
{
	int ret;
	int cflags;
	regex_t *regexp = p_new(pool_datastack_create(), regex_t, 1);

	if ( cmp == &i_octet_comparator ) 
		cflags =  REG_EXTENDED | REG_NOSUB;
	else if ( cmp ==  &i_ascii_casemap_comparator )
		cflags =  REG_EXTENDED | REG_NOSUB | REG_ICASE;
	else
		return NULL;

	if ( (ret=regcomp(regexp, key, cflags)) != 0 ) {
    	/* FIXME: Do something useful, i.e. report error somewhere */
		return NULL;
	}

	return regexp;
}

static bool mtch_regex_match
(const struct sieve_match_type *mtch ATTR_UNUSED, 
	const struct sieve_comparator *cmp ATTR_UNUSED, const char *val, 
	size_t val_size ATTR_UNUSED, const char *key ATTR_UNUSED, 
	size_t key_size ATTR_UNUSED, void *key_context)
{
	return ( regexec((regex_t *) key_context, val, 0, NULL, 0) == 0 );
}

void mtch_regex_match_deinit
	(const struct sieve_match_type *mtch ATTR_UNUSED, void *key_context)
{
	regfree((regex_t *) key_context);
}

/* Load extension into validator */

static bool ext_regex_validator_load(struct sieve_validator *validator)
{
	sieve_match_type_register
		(validator, &regex_match_type, ext_my_id); 

	return TRUE;
}

/* Load extension into interpreter */

static bool ext_regex_interpreter_load
	(struct sieve_interpreter *interpreter)
{
	sieve_match_type_extension_set
		(interpreter, ext_my_id, &regex_match_extension);

	return TRUE;
}


