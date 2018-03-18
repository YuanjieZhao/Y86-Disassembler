
#include <stdio.h>
#include <unistd.h>
#include "printRoutines.h"

// You probably want to create a number of printing routines in this file.
// Put the prototypes in printRoutines.h


/*********************************************************************
   Details on print formatting can be found by reading the man page
   for printf. The formatting rules for the disassembler output are
   given below.  Basically it should be the case that if you take the
   output from your disassembler, remove the initial address
   information, and then take the resulting file and load it into a
   Y86-64 simulator the object code produced by the simulator should
   match what your program read.  (You may have to use a .pos
   directive to indicate the offset to where the code starts.) If the
   simulator reports an assembler error then you need to fix the
   output of your dissassembler so that it is acceptable to the
   simulator.)

   The printing rules are as follows:
   
     1) Each line is to begin with the hex printed value of the
        address followed immediately by a ": ". Leading zeros are to
        be printed for the address, which is 8 bytes long.

     2) After the ": " there are to be 22 characters before the start
        of the printing of the instruction mnemonic. In those 22
        characters the program is to print, left justified, the hex
        representation of the memory values corresponding to the
        assembler instruction and operands that were printed.

     3) The instruction field follows, and is 8 characters long. The
        instruction is to be printed left justified in those 8
        characters (%-8s). All instructions are to be printed in lower
        case.

     4) After the instruction field the first operand is to be
        printed. No extra space is needed, since the 8 characters for
        instructions is enough to leave a space between instruction
        and operands.
 
     5) If there is a second operand then there is to be a comma
        immediately after the first operand (no spaces) and then a
        single space followed by the second operand.

     6) The rules for printing operands are as follows: 

         a) Registers: A register is to be printed with the % sign and
            then its name. (.e.g. %rsp, %rax etc) Register names are
            printed in lower case.
 
         b) All numbers are to be printed in hex with the appropriate
            leading 0x. Leading zeros are to be suppressed. A value of
            zero is to be printed as "0x0". The numbers are assumed to
            be unsigned.

         c) A base displacement address is to be printed as D(reg)
            where the printing of D follows the rules in (b), and the
            printing of reg follows the rules in (a). Note there are
            no spaces between the D and "(" or between reg and the "("
            or ")".
        
         d) An address such as that used by a call or jump is to be
            printed as in (b). For these instructions no "$" is
            required.

         e) A constant (immediate value), such as that used by irmovq
            is to be printed as a number in (b) but with a "$"
            immediately preceeding the 0x without any spaces.
           
     7) The unconditional move instruction is to be printed as rrmovq,
        while the conditional move is to be printed as cmovXX, where
        XX is the condition (e.g., cmovle).

     8) The mnemonics for the instruction are to conform to those
        described in the textbook and lecture slides.

     9) The arguments for the format string in the example printing
        are strictly for illustrating the spacing. You are free to
        construct the output however you want.
 
********************************************************************************/
 
/* This is a function to demonstrate how to do print formatting. It
 * takes the file descriptor the I/O is to be done to. You are not
 * required to use the same type of printf formatting, but you must
 * produce the same result.
 */
int samplePrint(FILE *out) {

  int res = 0;

  unsigned long addr = 0x1016;
  char * r1 = "%rax";
  char * r2 = "%rdx";
  char * inst1 = "rrmovq";
  char * inst2 = "jne";
  char * inst3 = "irmovq";
  char * inst4 = "mrmovq";
  unsigned long destAddr = 8193;
  
  res += fprintf(out, "%016lx: %-22s%-8s%s, %s\n", 
		 addr, "2002", inst1, r1, r2);

  addr += 2;
  res += fprintf(out, "%016lx: %-22s%-8s%#lx\n", 
		 addr, "740120000000000000", inst2, destAddr);

  addr += 9;
  res += fprintf(out, "%016lx: %-22s%-8s$%#lx, %s\n", 
		 addr, "30F21000000000000000", inst3, 16L, r2);

  addr += 10;
  res += fprintf(out, "%016lx: %-22s%-8s%#lx(%s), %s\n", 
		 addr, "50020000010000000000", inst4, 65536L, r2, r1); 
  
  addr += 10;
  res = fprintf(out, "%016lx: %-22s%-8s%s, %s\n", 
		addr, "2020", inst1, r2, r1);

  return res;
}  
  
// return the string representation of given register encoded in integer
const char* get_reg_name (uint8_t reg){
  switch (reg){
    case 0:
            return "%rax";
    case 1:
            return "%rcx";
    case 2:
            return "%rdx";
    case 3:
            return "%rbx";
    case 4:
            return "%rsp";
    case 5:
            return "%rbp";
    case 6:
            return "%rsi";
    case 7:
            return "%rdi";
    case 8:
            return "%r8";
    case 9:
            return "%r9";
    case 0xA:
            return "%r10";
    case 0xB:
            return "%r11";
    case 0xC:
            return "%r12";
    case 0xD:
            return "%r13";
    case 0xE:
            return "%r14";
    default:
            return "error: invalid register, this line shouldn't be printed\n";
    }
}

// store memory value (in integer representation) of current instruction to buffer
void get_inst_mem_val(char* buffer, inst_t inst){
  switch(inst.type){
    case HALT: case NOP: case RET:
        buffer[0] = inst.opcode;
        break;

    case OPQ: case CMOVXX: case PUSHQ: case POPQ:
        buffer[0] = inst.opcode;
        buffer[1] = inst.ra << 4 | inst.rb;
        break;

    case JXX: case CALL:
        buffer[0] = inst.opcode;
        convert_imm_val_to_byte_array(buffer, inst.imm_val, 1);
        break;

    case IRMOVQ: case RMMOVQ: case MRMOVQ:
        buffer[0] = inst.opcode;
        buffer[1] = inst.ra << 4 | inst.rb;
        convert_imm_val_to_byte_array(buffer, inst.imm_val, 2);
        break;

    case INVALID:
        convert_imm_val_to_byte_array(buffer, inst.imm_val, 0);
        break;
  }
}

// store an 8-byte value (in big endian) into a byte array buffer starting at start_pos in little endian
void convert_imm_val_to_byte_array(char* buffer, uint64_t imm_val, int start_pos){
  for (uint8_t i = 0; i < 8; i++){
    buffer[start_pos + i] = (imm_val >> (8 * i) & 0xFF);
  }
}

// print current address, memory value and assembly of given instruction to out file
void print_assembly (inst_t inst, long currAddr, FILE* out){
  char* inst_name;

  // store memory value of current instion in mem_val using hex string representation 
  char buffer[11] = {0};          // 10 bytes for longest instruction + 1 byte for end of string character = 11 bytes
  get_inst_mem_val(buffer, inst); // each buffer element contains two digits of memory value in integer representation
  char mem_val[22] = {0};         // ensure mem_val is large enough to contain end of string character
  for (int i = 0; i < inst.size; i++){
    // each buffer element is converted to a two-character hex string and stored in two slots of mem_val
    // use "unsigned char" to ensure sprintf use unsigned extension when converting memory value to hex string representation
    sprintf(mem_val+2*i, "%02x", (unsigned char) buffer[i]); 
  }
  
  // print current instruction to output file
  switch(inst.type) {
    case HALT:
            fprintf(out, "%016lx: %-22s%-8s\n", currAddr, mem_val, "halt");
            break;
    case NOP:
            fprintf(out, "%016lx: %-22s%-8s\n", currAddr, mem_val, "nop");
            break;
    case RET:
            fprintf(out, "%016lx: %-22s%-8s\n", currAddr, mem_val, "ret");
            break;


    case IRMOVQ:
            fprintf(out, "%016lx: %-22s%-8s$%#lx, %s\n", currAddr, mem_val, "irmovq", inst.imm_val, get_reg_name(inst.rb));
            break;

    case RMMOVQ:
            fprintf(out, "%016lx: %-22s%-8s%s, %#lx(%s)\n", currAddr, mem_val, "rmmovq", get_reg_name(inst.ra), inst.imm_val, get_reg_name(inst.rb)); 
            break;

    case MRMOVQ:
            fprintf(out, "%016lx: %-22s%-8s%#lx(%s), %s\n", currAddr, mem_val, "mrmovq", inst.imm_val, get_reg_name(inst.rb), get_reg_name(inst.ra)); // rb is before ra in the assembly of mrmovq
            break;

    case CALL:
            fprintf(out, "%016lx: %-22s%-8s%#lx\n", currAddr, mem_val, "call", inst.imm_val);
            break;

    case CMOVXX:
            switch(inst.cmov_type) {
              case RRMOVQ:
                      inst_name = "rrmovq";
                      break;
              case CMOVLE:
                      inst_name = "cmovle";
                      break;
              case CMOVL:
                      inst_name = "cmovl";
                      break;
              case CMOVE:
                      inst_name = "cmove";
                      break;
              case CMOVNE:
                      inst_name = "cmovne";
                      break;
              case CMOVGE:
                      inst_name = "cmovge";
                      break;
              case CMOVG:
                      inst_name = "cmovg";
                      break;
            }

            fprintf(out, "%016lx: %-22s%-8s%s, %s\n", currAddr, mem_val, inst_name, get_reg_name(inst.ra), get_reg_name(inst.rb));
            break;
    case OPQ:
            switch(inst.opq_type) {
              case ADD:
                      inst_name = "addq";
                      break;
              case SUB:
                      inst_name = "subq";
                      break;
              case AND:
                      inst_name = "andq";
                      break;
              case XOR:
                      inst_name = "xorq";
                      break;
            }
            fprintf(out, "%016lx: %-22s%-8s%s, %s\n", currAddr, mem_val, inst_name, get_reg_name(inst.ra), get_reg_name(inst.rb));
            break;

    case JXX:
            switch(inst.jump_type) {
              case JMP:
                      inst_name = "jmp";
                      break;
              case JLE:
                      inst_name = "jle";
                      break;
              case JL:
                      inst_name = "jl";
                      break;
              case JE:
                      inst_name = "je";
                      break;
              case JNE:
                      inst_name = "jne";
                      break;
              case JGE:
                      inst_name = "jge";
                      break;
              case JG:
                      inst_name = "jg";
                      break;
            }
            fprintf(out, "%016lx: %-22s%-8s%#lx\n", currAddr, mem_val, inst_name, inst.imm_val);
            break;

    case PUSHQ:
            inst_name = "pushq";
            fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, mem_val, inst_name, get_reg_name(inst.ra));
            break;

    case POPQ:
            inst_name = "popq";
            fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, mem_val, inst_name, get_reg_name(inst.ra));
            break;

    default:
            // invalid instruction, already handled in fetch_instruction
            break;
  }
}