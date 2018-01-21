#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

#define INITIAL_SIZE 5
#define SCALING_FACTOR 2

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    /* YOUR CODE HERE */
    Symbol* symbol = (Symbol*) malloc(INITIAL_SIZE * sizeof(Symbol));
    if (symbol == NULL) {
      allocation_failed();
    }
    SymbolTable* table_strct = (SymbolTable*) malloc(sizeof(SymbolTable));
    if (table_strct == NULL) {
      allocation_failed();
    }
    *table_strct = (SymbolTable) {symbol, 0, INITIAL_SIZE, mode};
    return (table_strct);
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
    /* YOUR CODE HERE */
    int i = 0;
    Symbol* symbol = table -> tbl;
    for (; i < table -> len; i++) {
      free(symbol -> name);
      symbol++;
    }
    free(table -> tbl);
    free(table);
}

/* A suggested helper function for copying the contents of a string. */
static char* create_copy_of_str(const char* str) {
    size_t len = strlen(str) + 1;
    char *buf = (char *) malloc(len);
    if (!buf) {
       allocation_failed();
    }
    strncpy(buf, str, len); 
    return buf;
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
    /* YOUR CODE HERE */
  if (table -> mode == SYMTBL_UNIQUE_NAME) {
    int i = 0;
    Symbol* sym = table -> tbl;
    for (; i < table -> len; i++) {
      if (strcmp((sym -> name), name) == 0) {
        name_already_exists(name);
        return -1;
      }
      sym++;
    }
  }
  if (addr % 4 != 0) {
    addr_alignment_incorrect();
    return -1;
  }

  if (table -> len >= table -> cap) {
    table -> cap *= SCALING_FACTOR;
    table -> tbl = (Symbol*) realloc(table -> tbl, (table -> cap) * sizeof(Symbol));
    if (table -> tbl == NULL) {
      allocation_failed();
    }
  }
  int x;
  Symbol* each_sym = table -> tbl;
  for (x = 0; x < (table -> len); x++) {
    each_sym++;
  }
  /* must account for the size of each char primitive  on different architectures */
  (each_sym -> name) = malloc((strlen(name) + 1) * sizeof(char));
  if (each_sym + 1 == NULL) {
    allocation_failed();
  }
  each_sym -> name = create_copy_of_str(name);
  each_sym -> addr = addr;
  table -> len += 1;
  return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    /* YOUR CODE HERE */
  int j = 0;
  Symbol* nme = table -> tbl;
  for (; j < table -> len; j++) {
    if (strcmp((nme -> name), name) == 0) {
      return (nme -> addr);
    }
    nme++;
  }
  return -1;
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    /* YOUR CODE HERE */
  int i = 0;
  Symbol* type_table = table -> tbl;
  for (; i < table -> len; i++) {
    write_symbol(output, type_table -> addr, type_table -> name);
    type_table++;
  }
}
