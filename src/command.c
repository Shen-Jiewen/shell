#include <stdio.h>
#include <string.h>
#include "command.h"
#include "uart.h"

#define MAX_COMMANDS 10
#define MAX_ALIASES 10

typedef struct {
    char name[32];
    command_func_t func;
} command_entry_t;

typedef struct {
    char alias[32];
    char command_name[32];
} alias_entry_t;

static command_entry_t command_table[MAX_COMMANDS];
static alias_entry_t alias_table[MAX_ALIASES];
static int command_count = 0;
static int alias_count = 0;

void register_command(const char *name, command_func_t func) {
    if (command_count < MAX_COMMANDS) {
        strncpy(command_table[command_count].name, name, sizeof(command_table[command_count].name) - 1);
        command_table[command_count].func = func;
        command_count++;
    } else {
        uart_send("Error: Command table full.\n");
    }
}

void register_alias(const char *alias, const char *command_name) {
    if (alias_count < MAX_ALIASES) {
        strncpy(alias_table[alias_count].alias, alias, sizeof(alias_table[alias_count].alias) - 1);
        strncpy(alias_table[alias_count].command_name, command_name, sizeof(alias_table[alias_count].command_name) - 1);
        alias_count++;
    } else {
        uart_send("Error: Alias table full.\n");
    }
}

static const char *resolve_alias(const char *alias) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(alias_table[i].alias, alias) == 0) {
            return alias_table[i].command_name;
        }
    }
    return alias; // 如果找不到别名，返回原始命令
}

void execute_command(const char *input) {
    char *argv[10];   // 支持最多10个参数
    int argc = 0;

    char input_copy[128];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    char *token = strtok(input_copy, " ");
    while (token != NULL && argc < 10) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        return; // 没有输入有效命令
    }

    const char *command_name = resolve_alias(argv[0]);
    for (int i = 0; i < command_count; i++) {
        if (strcmp(command_table[i].name, command_name) == 0) {
            command_table[i].func(argc, argv);
            return;
        }
    }

    uart_send("Error: Command not found.\n");
}

int get_command_count() {
    return command_count;
}

const char *get_command_name(int index) {
    if (index >= 0 && index < command_count) {
        return command_table[index].name;
    }
    return NULL;
}
