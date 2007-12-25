/* Extension variables 
 * -------------------
 *
 * Authors: Stephan Bosch
 * Specification: draft-ietf-sieve-variables-08.txt
 * Implementation: skeleton
 * Status: under development
 *
 */

#include "lib.h"
#include "unichar.h"

#include "sieve-extensions.h"
#include "sieve-commands.h"
#include "sieve-validator.h"

#include <ctype.h>

/* Forward declarations */

static bool ext_variables_load(int ext_id);
static bool ext_variables_validator_load(struct sieve_validator *validator);

/* Extension definitions */

static int ext_my_id;
	
struct sieve_extension variables_extension = { 
	"variables", 
	ext_variables_load,
	ext_variables_validator_load, 
	NULL, NULL, NULL, 
	SIEVE_EXT_DEFINE_NO_OPCODES, 
	NULL 
};

static bool ext_variables_load(int ext_id) 
{
	ext_my_id = ext_id;
	return TRUE;
}

/* New argument */

static bool arg_variable_string_validate
	(struct sieve_validator *validator, struct sieve_ast_argument **arg, 
		struct sieve_command_context *context);

const struct sieve_argument variable_string_argument =
	{ "@variable-string", NULL, arg_variable_string_validate, NULL, NULL };

static bool arg_variable_string_validate
(struct sieve_validator *validator, struct sieve_ast_argument **arg, 
		struct sieve_command_context *cmd)
{
	bool result = TRUE;
	enum { ST_NONE, ST_OPEN, ST_VARIABLE, ST_CLOSE } 
		state = ST_NONE;
	string_t *str = sieve_ast_argument_str(*arg);
	string_t *tmpstr, *newstr = NULL;
	const char *p, *mark, *strstart, *substart = NULL;
	const char *strval = (const char *) str_data(str);
	const char *strend = strval + str_len(str);

	T_FRAME(		
		tmpstr = t_str_new(32);	
			
		p = strval;
		strstart = p;
		while ( result && p < strend ) {
			switch ( state ) {
			case ST_NONE:
				if ( *p == '$' ) {
					substart = p;
					state = ST_OPEN;
				}
				p++;
				break;
			case ST_OPEN:
				if ( *p == '{' ) {
					state = ST_VARIABLE;
					p++;
				} else 
					state = ST_NONE;
				break;
			case ST_VARIABLE:
				mark = p;
				
				if ( p < strend ) {
					if (*p == '_' || isalpha(*p) ) {
						p++;
					
						while ( p < strend && (*p == '_' || isalnum(*p)) ) {
							p++;
						}
					} else if ( isdigit(*p) ) {
						unsigned int num_variable = *p - '0';
						p++;
						
						while ( p < strend && isdigit(*p) ) {
							num_variable = num_variable*10 + (*p - '0');
							p++;
						} 
					}
				} 			
				
				break;
			case ST_CLOSE:
				if ( *p == '}' ) {				
					/* We now know that the substitution is valid */	
					
					if ( newstr == NULL ) {
						newstr = str_new(sieve_ast_pool((*arg)->ast), str_len(str)*2);
					}
					
					str_append_n(newstr, strstart, substart-strstart);
					str_append_str(newstr, tmpstr);
					
					strstart = p + 1;
					substart = strstart;
				}
				state = ST_NONE;
				p++;	
			}
		}
	);
	
	if ( newstr != NULL ) {
		if ( strstart != strend )
			str_append_n(newstr, strstart, strend-strstart);	
	
		sieve_ast_argument_str_set(*arg, newstr);
	}
	
	return sieve_validator_argument_activate_super
		(validator, cmd, *arg, TRUE);
}

/* Load extension into validator */

static bool ext_variables_validator_load
	(struct sieve_validator *validator ATTR_UNUSED)
{
	sieve_validator_argument_override(validator, SAT_VAR_STRING, 
		&variable_string_argument); 
	return TRUE;
}
