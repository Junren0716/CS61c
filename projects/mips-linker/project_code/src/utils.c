
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

static const char* output_file = NULL;

int is_log_file_set() {
    return output_file != NULL;
}

void set_log_file(const char* filename) {
    if (filename) {
        output_file = filename;
        unlink(filename);
    } else {
        output_file = NULL;
    }
}

void write_to_log(char* fmt, ...) {
    va_list args;

    if (output_file) {
        FILE* f = fopen(output_file, "a");
        if (!f) {
            return;
        }
        
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);
        fclose(f);
    } else {
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

void log_inst(const char* name, char** args, int num_args) {
    if (output_file) {
        FILE* f = fopen(output_file, "a");
        if (!f) {
            return;
        }
        
        fprintf(f, "%s", name);
        for (int i = 0; i < num_args; i++) {
            fprintf(f, " %s", args[i]);
        }
        fprintf(f, "\n");
        fclose(f);
    } else {
        fprintf(stderr, "%s", name);
        for (int i = 0; i < num_args; i++) {
            fprintf(stderr, " %s", args[i]);
        }
        fprintf(stderr, "\n");
    }
}
