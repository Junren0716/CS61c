
int is_log_file_set();

void set_log_file(const char* filename);

void write_to_log(char* fmt, ...);

void log_inst(const char* name, char** args, int num_args);