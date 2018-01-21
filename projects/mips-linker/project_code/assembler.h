#ifndef ASSEMBLER_H
#define ASSEMBLER_H

int assemble(const char* in_name, const char* tmp_name, const char* out_name);

int pass_one(FILE *input, FILE* output, SymbolTable* symtbl);

int pass_two(FILE *input, FILE* output, SymbolTable* symtbl, SymbolTable* reltbl);

#endif
