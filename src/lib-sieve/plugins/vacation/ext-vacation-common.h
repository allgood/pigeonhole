/* Copyright (c) 2002-2010 Pigeonhole authors, see the included COPYING file
 */

#ifndef __EXT_VACATION_COMMON_H
#define __EXT_VACATION_COMMON_H

#include "sieve-common.h"

/*
 * Extension configuration
 */

#define EXT_VACATION_DEFAULT_PERIOD (7*24*60*60)
#define EXT_VACATION_DEFAULT_MIN_PERIOD (24*60*60)
#define EXT_VACATION_DEFAULT_MAX_PERIOD 0
 
struct ext_vacation_config {
	unsigned int min_period;
	unsigned int max_period;
	unsigned int default_period;
};

/* 
 * Commands 
 */

extern const struct sieve_command_def vacation_command;

/* 
 * Operations 
 */

extern const struct sieve_operation_def vacation_operation;

/* 
 * Extension 
 */

extern const struct sieve_extension_def vacation_extension;

bool ext_vacation_load
	(const struct sieve_extension *ext, void **context);
void ext_vacation_unload
	(const struct sieve_extension *ext);


#endif /* __EXT_VACATION_COMMON_H */
