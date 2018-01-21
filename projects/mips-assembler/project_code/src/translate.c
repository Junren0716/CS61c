#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tables.h"
#include "translate_utils.h"
#include "translate.h"

/* SOLUTION CODE BELOW */
const int TWO_POW_SEVENTEEN = 131072;    // 2^17

/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and blt pseudoinstructions. Your pseudoinstruction 
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are 
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number 
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into 
        a lui-ori pair.
        

   For move, blt, bgt, and traddu:
    - your expansion should use the fewest number of instructions possible.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that 
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {
    char* zero_reg = "$0";
    /* Increment with every call to fprintf */
    int num_instrct = 0;
    if (strcmp(name, "li") == 0) {
      /* YOUR CODE HERE */
      if (num_args == 2) {
        long int signed_num;
        long int lower = -2147483648;  /* -(2^31) */
        long int upper = 4294967295;   /* +(2^32) - 1, incase unsigned */
        int x = translate_num(&signed_num, args[1], lower, upper);
        if (x == 0) {
          int low = -32768;    /* -(2^15) */
          int high = 65536;    /* +(2^16), incase unsigned */
          if ((signed_num >= low && signed_num <= ((low * -1) - 1)) || (signed_num >= 0 && signed_num <= (high - 1))) {
            char* adu = "addiu";
            fprintf(output, "%s %s, %s, %ld\n", adu, args[0], zero_reg, signed_num);
            num_instrct += 1;
            return num_instrct;
          }
          long int top_half = signed_num >> 16;
          long int bottom_half = signed_num & 0xFFFF;
          fprintf(output, "lui %s, %ld\n", args[0], top_half);
          num_instrct += 1;
          fprintf(output, "ori %s, %s, %ld\n", args[0], args[0], bottom_half);
          num_instrct += 1;
          return num_instrct;
        }
      }
      return 0;
    } else if (strcmp(name, "move") == 0) {
      /* YOUR CODE HERE */
      if (num_args == 2) {
        char* add = "add";
        fprintf(output, "%s %s, %s, %s\n", add, args[0], args[1], zero_reg);
        num_instrct += 1;
        return num_instrct;
      }
      return 0;
    } else if (strcmp(name, "blt") == 0) {
      /* YOUR CODE HERE */
      if (num_args == 3) {
        /* Use $at for intermediate code */
        char* less_than = "slt, $at";
        char* if_not_equal = "bne, $at";
        fprintf(output, "%s, %s, %s\n", less_than, args[0], args[1]);
        num_instrct += 1;
        fprintf(output, "%s, %s, %s\n", if_not_equal, zero_reg, args[2]);
        num_instrct += 1;
        return num_instrct;
      }
      return 0;  
    } else if (strcmp(name, "bgt") == 0) {
      /* YOUR CODE HERE */
      if (num_args == 3) {
        /* Use $at for intermediate code */
        char* greater_than = "sgt, $at";
        char* not_equal = "bne, $at";
        fprintf(output, "%s, %s, %s\n", greater_than, args[0], args[1]);
        num_instrct += 1;
        fprintf(output, "%s, %s, %s\n", not_equal, zero_reg, args[2]);
        num_instrct += 1;
        return num_instrct;
      }
      return 0;
    } else if (strcmp(name, "traddu") == 0) {
      /* YOUR CODE HERE */
      if (num_args == 3) {
        char* add_unsigned = "addu";
        char* a_t = "$at";
        fprintf(output, "%s %s, %s, %s\n", add_unsigned, a_t, args[1], args[2]);
        num_instrct += 1;
        fprintf(output, "%s %s, %s, %s\n", add_unsigned, args[3], a_t, args[3]);
        num_instrct += 1;
        return num_instrct;
      }
      return 0;       
    } 
    write_inst_string(output, name, args, num_args);
    return 1;
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write 
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

   Some function declarations for the write_*() functions are provided in translate.h, and you MUST implement these
   and use these as function headers. You may use helper functions, but you still must implement
   the provided write_* functions declared in translate.h.

   Returns 0 on success and -1 on error. 
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args, uint32_t addr,
    SymbolTable* symtbl, SymbolTable* reltbl) {
    if (strcmp(name, "addu") == 0)       return write_rtype (0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0)    return write_rtype (0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0)   return write_rtype (0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0)  return write_rtype (0x2b, output, args, num_args);
    else if (strcmp(name, "sll") == 0)   return write_shift (0x00, output, args, num_args);
    /* YOUR CODE HERE */
    else if (strcmp(name, "jr") == 0)    return write_jr (0x08, output, args, num_args);
    else if (strcmp(name, "addiu") == 0)  return write_addiu (0x09, output, args, num_args);
    else if (strcmp(name, "ori") == 0)  return write_ori (0x0d, output, args, num_args);
    else if (strcmp(name, "lui") == 0)   return write_lui (0x0f, output, args, num_args);
    else if (strcmp(name, "lb") == 0)   return write_mem (0x20, output, args, num_args);
    else if (strcmp(name, "lbu") == 0)  return write_mem (0x24, output, args, num_args);
    else if (strcmp(name, "lw") == 0)   return write_mem (0x23, output, args, num_args);
    else if (strcmp(name, "sb") == 0)   return write_mem (0x28, output, args, num_args);
    else if (strcmp(name, "sw") == 0)  return write_mem (0x2b, output, args, num_args);
    else if (strcmp(name, "beq") == 0)   return write_branch (0x04, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "bne") == 0)   return write_branch (0x05, output, args, num_args, addr, symtbl);
    else if (strcmp(name, "j") == 0)  return write_jump (0x02, output, args, num_args, addr, reltbl);
    else if (strcmp(name, "jal") == 0)   return write_jump (0x03, output, args, num_args, addr, reltbl);
    else                                 return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args == 3) {
      int rd = translate_reg(args[0]);
      int rs = translate_reg(args[1]);
      int rt = translate_reg(args[2]);
      if (rd >= 0 && rs >= 0 && rt >= 0) {
        int op_code = (000000 << 26);
        rs = (rs << 21);
        rt = (rt << 16);
        rd = (rt << 11);
        int shamt = (00000 << 6);
        uint32_t instruction = op_code | rs | rt | rd | shamt | funct;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

/* A helper function for writing shift instructions. You should use 
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_shift(uint8_t funct, FILE* output, char** args, size_t num_args) {
  // Perhaps perform some error checking?
    if (num_args == 3) {
      long int shamt;
      int rd = translate_reg(args[0]);
      int rt = translate_reg(args[1]);
      int err = translate_num(&shamt, args[2], 0, 31);
      if (rt >= 0 && rd >= 0 && err >= 0) {
        int op_code = (000000 << 26);
        int rs = (00000 << 21);
        rt = (rt << 16);
        rd = (rd << 11);
        shamt = (shamt << 6);
        uint32_t instruction = op_code | rs | rt | rd | shamt | funct;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

/* The rest of your write_*() functions below */

int write_jr(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args == 1) {
      int rs = translate_reg(args[0]);
      if (rs >= 0) {
        int op_code = (000000 << 26);
        rs = (rs << 21);
        int rt = (00000 << 16);
        int rd = (00000 << 11);
        int shamt = (00000 << 6);
        uint32_t instruction = op_code | rs | rt | rd | shamt | funct;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

int write_addiu(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args == 3) {
      long int imm;
      int rt = translate_reg(args[0]);
      int rs = translate_reg(args[1]);
      int err = translate_num(&imm, args[2], INT16_MIN, INT16_MAX);
      if (rs >= 0 && rt >= 0 && err >= 0) {
        int op_code = (opcode << 26);
        rs = (rs << 21);
        rt = (rt << 16);
        if (imm < 0) {
          /* Keep only the last 16 bits and to account for unsigned */
          imm &= 0xFFFF;
        }
        uint32_t instruction = op_code | rs | rt | imm;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

int write_ori(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args == 3) {
      long int imm;
      int rt = translate_reg(args[0]);
      int rs = translate_reg(args[1]);
      int err = translate_num(&imm, args[2], 0, UINT16_MAX);
      if (rs >= 0 && rt >= 0 && err >= 0) {
        int op_code = (opcode << 26);
        rs = (rs << 21);
        rt = (rt << 16);
        uint32_t instruction = op_code | rs | rt | imm;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

int write_lui(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args == 2) {
      long int imm;
      int rt = translate_reg(args[0]);
      int err = translate_num(&imm, args[1], 0, UINT16_MAX);
      if (rt >= 0 && err >= 0) {
        int op_code = (opcode << 26);
        int rs = (00000 << 21);
        rt = (rt << 16);
        uint32_t instruction = op_code | rs | rt | imm;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

int write_mem(uint8_t opcode, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?
    if (num_args == 3) {
      long int imm;
      int rt = translate_reg(args[0]);
      int rs = translate_reg(args[2]);
      /* imm will be a signed number */
      int err = translate_num(&imm, args[1], INT16_MIN, INT16_MAX);
      if (rs >= 0 && rt >= 0 && err >= 0) {
        int op_code = (opcode << 26);
        /* flipped order here */
        rs = (rt << 21);
        rt = (rs << 16);
        if (imm < 0) {
          /* Keep only the last 16 bits */
          imm &= 0xFFFF;
        }
        uint32_t instruction = op_code | rs | rt | imm;
        write_inst_hex(output, instruction);
        return 0;
      }
    }
    return -1;
}

/*  A helper function to determine if a destination address
    can be branched to
*/
static int can_branch_to(uint32_t src_addr, uint32_t dest_addr) {
    int32_t diff = dest_addr - src_addr;
    return (diff >= 0 && diff <= TWO_POW_SEVENTEEN) || (diff < 0 && diff >= -(TWO_POW_SEVENTEEN - 4));
}


int write_branch(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* symtbl) {
    // Perhaps perform some error checking?
    if (num_args == 3) {
      int rs = translate_reg(args[0]);
      int rt = translate_reg(args[1]);
      int label_addr = get_addr_for_symbol(symtbl, args[2]);
      if (rs >= 0 && rt >= 0 && label_addr >= 0) {
        int op_code = (opcode << 26);
        rs = (rt << 21);
        rt = (rs << 16);
        /* PC = (PC + 4) + (imm * 4) --> imm = (PC - (PC - 4)) / 4 */
        int32_t offset = label_addr - addr - 4;
        if (offset % 4 == 0) {
          offset /= 4;
          /* Must account for only bottom 16 bits. */
          offset &= 0xFFFF;
          uint32_t instruction = op_code | rs | rt | offset;
          write_inst_hex(output, instruction);        
          return 0;
        }
      }
    }
    return -1;
}

int write_jump(uint8_t opcode, FILE* output, char** args, size_t num_args, uint32_t addr, SymbolTable* reltbl) {
    /* YOUR CODE HERE */
    if (num_args == 1) {
      char* place_to_go_to = args[0];
      add_to_table(reltbl, place_to_go_to, addr);
      int op_code = (opcode << 26);
      /* Needs to be relocated, so all bits should be 0 for imm */
      uint32_t instruction = op_code;
      write_inst_hex(output, instruction);
      return 0;
    }
    return -1;
}
