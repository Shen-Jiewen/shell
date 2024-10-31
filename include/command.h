#ifndef COMMAND_H
#define COMMAND_H

#define MAX_COMMANDS 50
#define MAX_ALIASES 20
#define MAX_ARGC 10
#define MAX_INPUT_SIZE 128

// 错误码宏定义
#define COMMAND_SUCCESS 0            // 操作成功
#define COMMAND_ERROR_TABLE_FULL -1  // 命令表已满
#define ALIAS_ERROR_TABLE_FULL -2    // 别名表已满
#define COMMAND_ERROR_NO_INPUT -3    // 没有有效输入
#define COMMAND_ERROR_NOT_FOUND -4   // 命令未找到

typedef void (*CommandFunction)(int argc, char *argv[]);

// 定义命令结构体
typedef struct Command {
    char name[32];                  // 命令名称
    CommandFunction function;       // 命令对应的执行函数
} Command;

// 定义别名结构体
typedef struct AliasEntry {
    char alias[MAX_INPUT_SIZE];       // 别名
    char command_name[MAX_INPUT_SIZE]; // 对应的命令名称
} AliasEntry;

// 定义命令管理器结构体
typedef struct CommandManager {
    Command command_table[MAX_COMMANDS];  // 命令表
    AliasEntry alias_table[MAX_ALIASES];  // 别名表
    int command_count;                    // 已注册的命令数量
    int alias_count;                      // 已注册的别名数量

    // 函数指针定义，作为“成员函数”来实现面向对象风格
    int (*register_command)(struct CommandManager* self, const char *name, CommandFunction func);
    int (*register_alias)(struct CommandManager* self, const char *alias, const char *command_name);
    int (*execute_command)(struct CommandManager* self, const char *input);
    int (*get_command_count)(struct CommandManager* self);
    const char *(*get_command_name)(struct CommandManager* self, int index);
} CommandManager;

// 获取命令管理器的单例指针
CommandManager* get_command_manager();

#endif // COMMAND_H
