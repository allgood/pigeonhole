/* Copyright (c) 2002-2009 Dovecot Sieve authors, see the included COPYING file
 */

#ifndef __TESTSUITE_RESULT_H
#define __TESTSUITE_RESULT_H

void testsuite_result_init(void);
void testsuite_result_deinit(void);

void testsuite_result_reset    
	(const struct sieve_runtime_env *renv);

struct sieve_result *testsuite_result_get(void);

struct sieve_result_iterate_context *testsuite_result_iterate_init(void);

bool testsuite_result_execute(const struct sieve_runtime_env *renv);

void testsuite_result_print
	(const struct sieve_runtime_env *renv ATTR_UNUSED);

#endif /* __TESTSUITE_RESULT_H */
