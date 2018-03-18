/* This file contains the prototypes and constants needed to use the
   routines defined in printRoutines.c
*/

#ifndef _PRINTROUTINES_H_
#define _PRINTROUTINES_H_

#include <stdio.h>

#include <stdint.h>
#include <inttypes.h>

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1

typedef enum y86_instruction_type {
	HALT, NOP, RET, CMOVXX, IRMOVQ, RMMOVQ, MRMOVQ, OPQ, JXX, CALL, PUSHQ, POPQ, INVALID
} inst_type_t;

typedef enum cmov_type {
	RRMOVQ, CMOVLE, CMOVL, CMOVE, CMOVNE, CMOVGE, CMOVG
} cmove_type_t;

typedef enum opq_type {
	ADD, SUB, AND, XOR
} opq_type_t;

typedef enum jump_type {
	JMP, JLE, JL, JE, JNE, JGE, JG
} jump_type_t;

typedef struct {
	inst_type_t type;
	uint8_t size;
	uint8_t opcode;
	uint8_t ra;
	uint8_t rb;
	uint64_t imm_val;	 		// immediate value in the instruction
	char* mem_val;				// memory value (in hex) of the instruction
	cmove_type_t cmov_type;
	opq_type_t	opq_type;
	jump_type_t	jump_type;
} inst_t;

int samplePrint(FILE *);
const char* get_reg_name (uint8_t reg);
void print_assembly (inst_t inst, long currAddr, FILE* out);
uint64_t get_8_bytes_from_array (uint8_t* src, int start_pos, int output_endianness);
void get_inst_mem_val(char* buffer, inst_t inst);
void convert_imm_val_to_byte_array(char* buffer, uint64_t imm_val, int start_pos);


#endif /* PRINTROUTINES */
