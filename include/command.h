#ifndef COMMAND_H
#define COMMAND_H

typedef void (*command_func_t)(int argc, char *argv[]);

void register_command(const char *name, command_func_t func);
void register_alias(const char *alias, const char *command_name);
void execute_command(const char *input);

int get_command_count();
const char *get_command_name(int index);

#endif // COMMAND_H
