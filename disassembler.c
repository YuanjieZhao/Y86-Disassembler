#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

inst_t fetch_instruction(FILE* inputStream, long currAddr, FILE* out);
long get_addr_of_next_non_zero_byte (FILE* inputStream, long currAddr, long file_length);


int main(int argc, char **argv) {
  
  FILE *machineCode, *outputFile;
  long currAddr = 0; // current reading position of a file

  // Verify that the command line has an appropriate number
  // of arguments

  if (argc < 3 || argc > 4) {
    printf("Usage: %s InputFilename OutputFilename [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to read, attempt to open it 
  // for reading and verify that the open did occur.
  machineCode = fopen(argv[1], "rb"); // r for read, b for binary

  if (machineCode == NULL) {
    printf("Failed to open %s: %s\n", argv[1], strerror(errno));
    return ERROR_RETURN;
  }

  // Second argument is the file to write, attempt to open it 
  // for writing and verify that the open did occur.
  outputFile = fopen(argv[2], "w");  // w for write

  if (outputFile == NULL) {
    printf("Failed to open %s: %s\n", argv[2], strerror(errno));
    fclose(machineCode);
    return ERROR_RETURN;
  }

  // If there is a 3rd argument present it is an offset so
  // convert it to a value. 
  if (4 == argc) {
    // See man page for strtol() as to why we check for errors by examining errno
    errno = 0;
    currAddr = strtol(argv[3], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      fclose(machineCode);
      fclose(outputFile);
      return ERROR_RETURN;
    }
  }

  printf("Opened %s, starting offset 0x%lX\n", argv[1], currAddr);
  printf("Saving output to %s\n", argv[2]);

  // Your code starts here.

  fseek(machineCode, 0, SEEK_END); // seek to end of file
  long file_length = ftell(machineCode); // get current file position (i.e. total number of bytes in the file) 
  fseek(machineCode, 0, SEEK_SET); // seek back to beginning of file

  // move the file reading postion to the first non-zero byte in machineCode
  currAddr = get_addr_of_next_non_zero_byte(machineCode, currAddr, file_length);
  fseek(machineCode, currAddr, SEEK_SET);

  // start disassembing
  inst_t inst;
  while(currAddr < file_length){
    inst = fetch_instruction(machineCode, currAddr, outputFile);
    if (inst.type != INVALID) {
      print_assembly(inst, currAddr, outputFile);
    }
    currAddr += inst.size;

    if (inst.type == HALT){
      // move the file reading postion to the next non-zero byte
      currAddr = get_addr_of_next_non_zero_byte(machineCode, currAddr, file_length);
      fseek(machineCode, currAddr, SEEK_SET);      
    }
  }
  
  fclose(machineCode);
  fclose(outputFile);
  return SUCCESS;
}




// fetch a single instruction from inputStream, eheck its invalidity, and return the instruction
// if the instruction is invalid, print the address, memory value and assemlby of it to out file
inst_t fetch_instruction(FILE* inputStream, long currAddr, FILE* out){
  uint8_t inst_buffer [10];
  uint16_t num_of_bytes_fetched;
  fread(inst_buffer, 1, 1, inputStream);
  inst_t inst;

  switch (inst_buffer[0]) {
    case 0x00:
          inst.type = HALT;
          inst.size = 1;
          inst.opcode = 0x00;
          break;
    case 0x10:
            inst.type = NOP;
            inst.size = 1;
            inst.opcode = 0x10;
            break;

    case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26:
            inst.type = CMOVXX;
            switch (inst_buffer[0])
            {
            case 0x20:
                    inst.cmov_type = RRMOVQ;
                    inst.opcode = 0x20;
                    break;
            case 0x21:
                    inst.cmov_type = CMOVLE;
                    inst.opcode = 0x21;
                    break;
            case 0x22:
                    inst.cmov_type = CMOVL;
                    inst.opcode = 0x22;
                    break;
            case 0x23:
                    inst.cmov_type = CMOVE;
                    inst.opcode = 0x23;
                    break;
            case 0x24:
                    inst.cmov_type = CMOVNE;
                    inst.opcode = 0x24;
                    break;
            case 0x25:
                    inst.cmov_type = CMOVGE;
                    inst.opcode = 0x25;
                    break;
            case 0x26:
                    inst.cmov_type = CMOVG;
                    inst.opcode = 0x26;
                    break;
            }

          inst.size = 2;
          // fetch remaining bytes of current instruction and check if invalid due to EOF
          // Note: file reading position is currently after opcode of current instruction 
          num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
          if (num_of_bytes_fetched < inst.size-1){
            inst.type = INVALID;
            break;
          }
          inst.ra = inst_buffer[1] >> 4 & 0x0F;
          inst.rb = inst_buffer[1] & 0x0F;
          if (inst.ra > 0xE || inst.rb > 0xE) { // inst.ra is unsigned, so no need to check inst.ra < 0
            inst.type = INVALID;
          }
          break;

    case 0x30:
        inst.type = IRMOVQ;
        inst.size = 10;
        inst.opcode = 0x30;

        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.ra = inst_buffer[1] >> 4 & 0x0F;
        inst.rb = inst_buffer[1] & 0x0F;
        if (inst.ra != 0xF || inst.rb > 0xE) {
          inst.type = INVALID;  
        }
        inst.imm_val = get_8_bytes_from_array(inst_buffer, 2, BIG_ENDIAN);
        break;

    case 0x40:
        inst.type = RMMOVQ;
        inst.size = 10;
        inst.opcode = 0x40;

        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }

        inst.ra = inst_buffer[1] >> 4 & 0x0F;
        inst.rb = inst_buffer[1] & 0x0F;
        if (inst.ra > 0xE || inst.rb > 0xE) {
          inst.type = INVALID;
        }
        inst.imm_val = get_8_bytes_from_array(inst_buffer, 2, BIG_ENDIAN);
        break;

    case 0x50:
        inst.type = MRMOVQ;
        inst.size = 10;
        inst.opcode = 0x50;

        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.ra = inst_buffer[1] >> 4 & 0x0F;
        inst.rb = inst_buffer[1] & 0x0F;
        if (inst.ra > 0xE || inst.rb > 0xE) {
          inst.type = INVALID;
          break;
        }

        inst.imm_val = get_8_bytes_from_array(inst_buffer, 2, BIG_ENDIAN);
        break;

    case 0x60: case 0x61: case 0x62: case 0x63:
        inst.type = OPQ;
        switch (inst_buffer[0])
        {
        case 0x60:
                inst.opq_type = ADD;
                inst.opcode = 0x60;
                break;
        case 0x61:
                inst.opq_type = SUB;
                inst.opcode = 0x61;
                break;
        case 0x62:
                inst.opq_type = AND;
                inst.opcode = 0x62;
                break;
        case 0x63:
                inst.opq_type = XOR;
                inst.opcode = 0x63;
                break;
        }

        inst.size = 2;
        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.ra = inst_buffer[1] >> 4 & 0x0F;
        inst.rb = inst_buffer[1] & 0x0F;
        if (inst.ra > 0xE || inst.rb > 0xE) {
                inst.type = INVALID;
        }
        break;

    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76:
        inst.type = JXX;
        switch (inst_buffer[0])
        {
        case 0x70:
                inst.jump_type = JMP;
                inst.opcode = 0x70;
                break;
        case 0x71:
                inst.jump_type = JLE;
                inst.opcode = 0x71;
                break;
        case 0x72:
                inst.jump_type = JL;
                inst.opcode = 0x72;
                break;
        case 0x73:
                inst.jump_type = JE;
                inst.opcode = 0x73;
                break;
        case 0x74:
                inst.jump_type = JNE;
                inst.opcode = 0x74;
                break;
        case 0x75:
                inst.jump_type = JGE;
                inst.opcode = 0x75;
                break;
        case 0x76:
                inst.jump_type = JG;
                inst.opcode = 0x76;
                break;
        }
        inst.size = 9;
        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.imm_val = get_8_bytes_from_array(inst_buffer, 1, BIG_ENDIAN);
        break;

    case 0x80:
        inst.type = CALL;
        inst.opcode = 0x80;
        inst.size = 9;
        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.imm_val = get_8_bytes_from_array(inst_buffer, 1, BIG_ENDIAN);
        break;        

    case 0x90:
        inst.type = RET;
        inst.opcode = 0x90;
        inst.size = 1;
        break;

    case 0xa0:
        inst.type = PUSHQ;
        inst.opcode = 0xa0;
        inst.size = 2;
        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.ra = inst_buffer[1] >> 4 & 0x0F;
        inst.rb = inst_buffer[1] & 0x0F;
        if (inst.ra > 0xE || inst.rb != 0xF) {
          inst.type = INVALID;
        }
        break;

    case 0xb0:
        inst.type = POPQ;
        inst.opcode = 0xb0;
        inst.size = 2;
        num_of_bytes_fetched = fread(inst_buffer+1, 1, inst.size-1, inputStream);
        if (num_of_bytes_fetched < inst.size-1){
          inst.type = INVALID;
          break;
        }
        inst.ra = inst_buffer[1] >> 4 & 0x0F;
        inst.rb = inst_buffer[1] & 0x0F;
        if (inst.ra > 0xE || inst.rb != 0xF) {
          inst.type = INVALID;
        }
        break;

    default:
        inst.type = INVALID;
        break;
  }

  // handle invalid instruction
  if (inst.type == INVALID){
    inst.size = 8;
    // move file reading position to the beginning of current instruction and read the next 8 bytes if available
    fseek(inputStream, currAddr, SEEK_SET);
    num_of_bytes_fetched = fread(inst_buffer, 1, inst.size, inputStream);

    // if actual instruction is shorter than 8 bytes due to EOF, update inst_size
    inst.size = num_of_bytes_fetched;

    // print invalid instruction
    char mem_val[8*2+1] = {0};
    if (num_of_bytes_fetched < 8){
      for (int i = 0; i < num_of_bytes_fetched; i++){
        sprintf(mem_val, "%02x", (unsigned char) inst_buffer[i]);
        fprintf(out, "%016lx: %-22s.byte %#02hhx\n", currAddr + i, mem_val, inst_buffer[i]);
      }
    } else {
      for (int i = 0; i < 8; i++){
        sprintf(mem_val+2*i, "%02x", (unsigned char) inst_buffer[i]);
      }
      fprintf(out, "%016lx: %-22s.quad %#lx\n", currAddr, mem_val, get_8_bytes_from_array(inst_buffer, 0, BIG_ENDIAN));
    }
  }

  return inst;
}


// return the address of the next non-zero byte from current address in inputStream
long get_addr_of_next_non_zero_byte (FILE* inputStream, long currAddr, long file_length){
  uint8_t read_val[1] = {0};
  while (currAddr < file_length) {
    fread(read_val, 1, 1, inputStream);
    if (read_val[0] != 0) break;
    currAddr += 1;
  }
  return currAddr;
}

// starting at given position of a byte array src, return the next 8 bytes as 
// a single integer in big or little endian as specified
uint64_t get_8_bytes_from_array (uint8_t* src, int start_pos, int output_endianness){
  uint64_t result = 0;
  if (output_endianness == BIG_ENDIAN){
    for (int i = 0; i < 8; i++){
      result = result | ((uint64_t) src[start_pos + i] << (8 * i));
    }
  } else {
    for (int i = 0; i < 8; i++){
      result = result | ((uint64_t) src[start_pos + (7-i)] << (8 * i));
    }
  }

  return result;
}