#ifndef SHELL_H
#define SHELL_H

#include "command.h"
#include "history.h"
#include "pal.h"
#include "log.h"  // 引入日志头文件

#define SHELL_VERSION "1.0.0"
#define INPUT_BUFFER_SIZE 128

typedef enum {
    EVENT_NONE,
    EVENT_KEY_ENTER,
    EVENT_KEY_BACKSPACE,
    EVENT_KEY_UP,
    EVENT_KEY_DOWN,
    EVENT_KEY_TAB,
    EVENT_KEY_CHAR
} ShellEvent;

typedef struct Shell {
    CommandManager *command_manager;   // 命令管理器
    HistoryManager *history_manager;   // 历史记录管理器
    PalInterface *pal;                 // 平台抽象层接口指针
    LogManager *log_manager;           // 日志管理器

    char input_buffer[INPUT_BUFFER_SIZE]; // 输入缓冲区
    int buffer_length;                 // 当前输入缓冲区的长度

    // 初始化 shell，包括平台、命令和历史管理器
    void (*init)(struct Shell *self);

    // shell 主循环
    void (*loop)(struct Shell *self);

    // 注册命令
    int (*register_command)(struct Shell *self, const char *name, CommandFunction func);

    // 事件处理器
    void (*handle_event)(struct Shell *self, ShellEvent event, int data);

} Shell;

// 创建并初始化 Shell 实例
Shell* create_shell();

#endif // SHELL_H
