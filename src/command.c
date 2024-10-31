#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "pal.h"

// 静态全局的命令管理器单例
static CommandManager command_manager = { .command_count = 0, .alias_count = 0 };

// 注册命令到命令管理器
int command_register_command(CommandManager* self, const char *name, CommandFunction func) {
    if (self->command_count >= MAX_COMMANDS) {
        return COMMAND_ERROR_TABLE_FULL; // 错误：命令表已满
    }

    Command *cmd = &self->command_table[self->command_count++];
    strncpy(cmd->name, name, sizeof(cmd->name) - 1);
    cmd->name[sizeof(cmd->name) - 1] = '\0'; // 确保以 '\0' 结尾
    cmd->function = func;
    return COMMAND_SUCCESS; // 成功
}

// 注册别名到别名表
int command_register_alias(CommandManager* self, const char *alias, const char *command_name) {
    if (self->alias_count >= MAX_ALIASES) {
        return ALIAS_ERROR_TABLE_FULL; // 错误：别名表已满
    }

    AliasEntry *entry = &self->alias_table[self->alias_count++];
    strncpy(entry->alias, alias, sizeof(entry->alias) - 1);
    entry->alias[sizeof(entry->alias) - 1] = '\0';
    strncpy(entry->command_name, command_name, sizeof(entry->command_name) - 1);
    entry->command_name[sizeof(entry->command_name) - 1] = '\0';
    return COMMAND_SUCCESS; // 成功
}

// 查找别名
static const char *command_resolve_alias(CommandManager* self, const char *alias) {
    for (int i = 0; i < self->alias_count; i++) {
        if (strcmp(self->alias_table[i].alias, alias) == 0) {
            return self->alias_table[i].command_name;
        }
    }
    return alias; // 如果找不到别名，返回原始命令
}

// 执行命令
int command_execute_command(CommandManager* self, const char *input) {
    char *argv[MAX_ARGC];
    int argc = 0;

    char input_copy[MAX_INPUT_SIZE];
    strncpy(input_copy, input, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\0'; // 确保以 '\0' 结尾

    char *token = strtok(input_copy, " ");
    while (token != NULL && argc < MAX_ARGC) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        return COMMAND_ERROR_NO_INPUT; // 错误：没有有效命令输入
    }

    const char *command_name = command_resolve_alias(self, argv[0]);
    for (int i = 0; i < self->command_count; i++) {
        if (strcmp(self->command_table[i].name, command_name) == 0) {
            self->command_table[i].function(argc, argv);
            return COMMAND_SUCCESS; // 成功
        }
    }

    return COMMAND_ERROR_NOT_FOUND; // 错误：命令未找到
}

// 获取已注册的命令数
int command_get_command_count(CommandManager* self) {
    return self->command_count;
}

// 获取指定索引处的命令名称
const char *command_get_command_name(CommandManager* self, int index) {
    if (index >= 0 && index < self->command_count) {
        return self->command_table[index].name;
    }
    return NULL; // 错误：索引超出范围
}

// 获取单例命令管理器的指针
CommandManager* get_command_manager() {
    command_manager.register_command = command_register_command;
    command_manager.register_alias = command_register_alias;
    command_manager.execute_command = command_execute_command;
    command_manager.get_command_count = command_get_command_count;
    command_manager.get_command_name = command_get_command_name;

    return &command_manager;
}
