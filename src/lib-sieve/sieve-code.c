#include <stdio.h>

#include "sieve-code.h"
#include "sieve-interpreter.h"

static bool sieve_code_dump_jmp(struct sieve_interpreter *interpreter);
static bool sieve_code_dump_jmptrue(struct sieve_interpreter *interpreter);
static bool sieve_code_dump_jmpfalse(struct sieve_interpreter *interpreter);
static bool sieve_code_dump_stop(struct sieve_interpreter *interpreter);
static bool sieve_code_dump_keep(struct sieve_interpreter *interpreter);
static bool sieve_code_dump_discard(struct sieve_interpreter *interpreter);

static bool sieve_code_execute_jmp(struct sieve_interpreter *interpreter);
static bool sieve_code_execute_jmptrue(struct sieve_interpreter *interpreter);
static bool sieve_code_execute_jmpfalse(struct sieve_interpreter *interpreter);
static bool sieve_code_execute_stop(struct sieve_interpreter *interpreter);
static bool sieve_code_execute_keep(struct sieve_interpreter *interpreter);
static bool sieve_code_execute_discard(struct sieve_interpreter *interpreter);

const struct sieve_opcode sieve_opcode_jmp = { sieve_code_dump_jmp, sieve_code_execute_jmp };
const struct sieve_opcode sieve_opcode_jmptrue = { sieve_code_dump_jmptrue, sieve_code_execute_jmptrue };
const struct sieve_opcode sieve_opcode_jmpfalse = { sieve_code_dump_jmpfalse, sieve_code_execute_jmpfalse };
const struct sieve_opcode sieve_opcode_stop = { sieve_code_dump_stop, sieve_code_execute_stop };
const struct sieve_opcode sieve_opcode_keep = { sieve_code_dump_keep, sieve_code_execute_keep };
const struct sieve_opcode sieve_opcode_discard = { sieve_code_dump_discard, sieve_code_execute_discard };

extern const struct sieve_opcode tst_address_opcode;
extern const struct sieve_opcode tst_header_opcode;
extern const struct sieve_opcode tst_exists_opcode;
extern const struct sieve_opcode tst_size_over_opcode;
extern const struct sieve_opcode tst_size_under_opcode;

const struct sieve_opcode *sieve_opcodes[] = {
  &sieve_opcode_jmp,
  &sieve_opcode_jmptrue, 
  &sieve_opcode_jmpfalse,
  &sieve_opcode_stop,
  &sieve_opcode_keep,
  &sieve_opcode_discard,

  &tst_address_opcode,
  &tst_header_opcode,
  &tst_exists_opcode,
  &tst_size_over_opcode,
  &tst_size_under_opcode
}; 

const unsigned int sieve_opcode_count =
	(sizeof(sieve_opcodes) / sizeof(sieve_opcodes[0]));

/* Code dump for core commands */

static bool sieve_code_dump_jmp(struct sieve_interpreter *interpreter)
{
	unsigned int pc = sieve_interpreter_program_counter(interpreter);
	int offset;
	
	if ( sieve_interpreter_read_offset_operand(interpreter, &offset) ) 
		printf("JMP %d [%08x]\n", offset, pc + offset);
	else
		return FALSE;
	
	return TRUE;
}	
		
static bool sieve_code_dump_jmptrue(struct sieve_interpreter *interpreter)
{	
	unsigned int pc = sieve_interpreter_program_counter(interpreter);
	int offset;
	
	if ( sieve_interpreter_read_offset_operand(interpreter, &offset) ) 
		printf("JMPTRUE %d [%08x]\n", offset, pc + offset);
	else
		return FALSE;
	
	return TRUE;
}

static bool sieve_code_dump_jmpfalse(struct sieve_interpreter *interpreter)
{	
	unsigned int pc = sieve_interpreter_program_counter(interpreter);
	int offset;
	
	if ( sieve_interpreter_read_offset_operand(interpreter, &offset) )
		printf("JMPFALSE %d [%08x]\n", offset, pc + offset);
	else
		return FALSE;
	
	return TRUE;
}	
	
static bool sieve_code_dump_stop(struct sieve_interpreter *interpreter ATTR_UNUSED)
{	
	printf("STOP\n");
	
	return TRUE;
}

static bool sieve_code_dump_keep(struct sieve_interpreter *interpreter ATTR_UNUSED)
{	
	printf("KEEP\n");
	
	return TRUE;
}

static bool sieve_code_dump_discard(struct sieve_interpreter *interpreter ATTR_UNUSED)
{	
	printf("DISCARD\n");
	
	return TRUE;
}

/* Code execution for core commands */

static bool sieve_code_execute_jmp(struct sieve_interpreter *interpreter)
{
	printf("JMP\n");
	if ( !sieve_interpreter_program_jump(interpreter, TRUE) )
		return FALSE;
	
	return TRUE;
}	
		
static bool sieve_code_execute_jmptrue(struct sieve_interpreter *interpreter)
{	
	if ( !sieve_interpreter_program_jump(interpreter,
		sieve_interpreter_get_test_result(interpreter)) )
		return FALSE;
		
	printf("JMPTRUE\n");
	
	return TRUE;
}

static bool sieve_code_execute_jmpfalse(struct sieve_interpreter *interpreter)
{	
	if ( !sieve_interpreter_program_jump(interpreter,
		!sieve_interpreter_get_test_result(interpreter)) )
		return FALSE;
		
	printf("JMPFALSE\n");
	
	return TRUE;
}	
	
static bool sieve_code_execute_stop(struct sieve_interpreter *interpreter ATTR_UNUSED)
{	
	printf(">> STOP\n");
	
	return FALSE;
}

static bool sieve_code_execute_keep(struct sieve_interpreter *interpreter ATTR_UNUSED)
{	
	printf(">> KEEP\n");
	
	return TRUE;
}

static bool sieve_code_execute_discard(struct sieve_interpreter *interpreter ATTR_UNUSED)
{	
	printf(">> DISCARD\n");
	
	return TRUE;
}

