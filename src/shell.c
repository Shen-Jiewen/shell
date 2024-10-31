#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shell.h"
#include "command.h"
#include "history.h"
#include "pal.h"

// 键码定义
#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_ENTER 10
#define KEY_BACKSPACE 127
#define KEY_TAB '\t'

// 命令函数声明
static void hello_command(int argc, char *argv[]);
static void list_command(int argc, char *argv[]);
static void reboot_command(int argc, char *argv[]);
static void clear_command(int argc, char *argv[]);

// 打印 Shell Logo 和版本信息
static void print_logo(Shell *self) {
    self->pal->uart_send("\n");
    self->pal->uart_send(" ____  _          _ _ \n");
    self->pal->uart_send("/ ___|| |__   ___| | |\n");
    self->pal->uart_send("\\___ \\| '_ \\ / _ \\ | |\n");
    self->pal->uart_send(" ___) | | | |  __/ | |\n");
    self->pal->uart_send("|____/|_| |_|\\___|_|_|\n");
    self->pal->uart_send("\n");

    char version_info[64];
    snprintf(version_info, sizeof(version_info), "Embedded Shell v%s\n", SHELL_VERSION);
    self->pal->uart_send(version_info);
}

// 初始化 Shell
static void shell_init(Shell *self) {
    self->pal->init();
    print_logo(self);

    // 初始化历史记录和命令
    self->history_manager->init(self->history_manager);

    // 注册命令及其别名
    self->command_manager->register_command(self->command_manager, "hello", hello_command);
    self->command_manager->register_command(self->command_manager, "list", list_command);
    self->command_manager->register_command(self->command_manager, "reboot", reboot_command);
    self->command_manager->register_command(self->command_manager, "clear", clear_command);

    self->command_manager->register_alias(self->command_manager, "ls", "list");
    self->command_manager->register_alias(self->command_manager, "rb", "reboot");
}

// 注册命令
static int shell_register_command(Shell *self, const char *name, CommandFunction func) {
    return self->command_manager->register_command(self->command_manager, name, func);
}

// 处理输入命令
static void process_input(Shell *self) {
    if (self->buffer_length > 0) {
        self->input_buffer[self->buffer_length] = '\0';
        self->history_manager->add(self->history_manager, self->input_buffer);

        if (strcmp(self->input_buffer, "exit") == 0) {
            self->pal->uart_send("Exiting shell...\n");
            exit(0);
        }
        self->command_manager->execute_command(self->command_manager, self->input_buffer);
    }
}

// 事件处理器
static void shell_handle_event(Shell *self, ShellEvent event, int data) {
    switch (event) {
        case EVENT_KEY_ENTER:
            self->pal->uart_send("\n");
            process_input(self);
            self->buffer_length = 0;
            memset(self->input_buffer, 0, sizeof(self->input_buffer));
            break;

        case EVENT_KEY_BACKSPACE:
            if (self->buffer_length > 0) {
                self->buffer_length--;
                self->pal->uart_send("\b \b");
            }
            break;

        case EVENT_KEY_UP: {
            const char *previous_command = self->history_manager->get_previous(self->history_manager);
            if (previous_command) {
                while (self->buffer_length > 0) {
                    self->pal->uart_send("\b \b");
                    self->buffer_length--;
                }
                strncpy(self->input_buffer, previous_command, sizeof(self->input_buffer) - 1);
                self->buffer_length = strlen(self->input_buffer);
                self->pal->uart_send(self->input_buffer);
            }
            break;
        }

        case EVENT_KEY_DOWN: {
            const char *next_command = self->history_manager->get_next(self->history_manager);
            if (next_command) {
                while (self->buffer_length > 0) {
                    self->pal->uart_send("\b \b");
                    self->buffer_length--;
                }
                strncpy(self->input_buffer, next_command, sizeof(self->input_buffer) - 1);
                self->buffer_length = strlen(self->input_buffer);
                self->pal->uart_send(self->input_buffer);
            }
            break;
        }

        case EVENT_KEY_CHAR:
            if (self->buffer_length < sizeof(self->input_buffer) - 1) {
                self->input_buffer[self->buffer_length++] = (char)data;
                self->pal->uart_send((char[]){(char)data, '\0'});
            }
            break;

        default:
            break;
    }
}

// Shell 主循环
static void shell_loop(Shell *self) {
    while (true) {
        self->pal->uart_send("shell> ");
        self->buffer_length = 0;
        memset(self->input_buffer, 0, sizeof(self->input_buffer));

        while (true) {
            int ch = self->pal->get_char();

            // 根据输入字符产生事件
            if (ch == KEY_ENTER) {
                self->handle_event(self, EVENT_KEY_ENTER, 0);
                break;
            } else if (ch == KEY_BACKSPACE) {
                self->handle_event(self, EVENT_KEY_BACKSPACE, 0);
            } else if (ch == 27) { // 方向键序列以 27 开头
                self->pal->get_char(); // 跳过 '['
                ch = self->pal->get_char();
                if (ch == KEY_UP) {
                    self->handle_event(self, EVENT_KEY_UP, 0);
                } else if (ch == KEY_DOWN) {
                    self->handle_event(self, EVENT_KEY_DOWN, 0);
                }
            } else {
                self->handle_event(self, EVENT_KEY_CHAR, ch);
            }
        }
    }
}

// 创建并初始化 Shell 实例
Shell* create_shell() {
    Shell *shell = (Shell*)malloc(sizeof(Shell));
    shell->command_manager = get_command_manager();
    shell->history_manager = get_history_manager();
    shell->pal = get_pal_interface();
    shell->init = shell_init;
    shell->loop = shell_loop;
    shell->register_command = shell_register_command;
    shell->handle_event = shell_handle_event;
    shell->buffer_length = 0;

    // 初始化 Shell，包括命令和历史管理器
    shell->init(shell);

    return shell;
}

// ========== 命令实现区域 ==========

// Hello 命令实现
static void hello_command(int argc, char *argv[]) {
    PalInterface *pal = get_pal_interface();
    if (argc > 1) {
        pal->uart_send("Hello, ");
        pal->uart_send(argv[1]);
        pal->uart_send("!\n");
    } else {
        pal->uart_send("Hello, World!\n");
    }
}

// List 命令实现
static void list_command(int argc, char *argv[]) {
    CommandManager *cm = get_command_manager();
    for (int i = 0; i < cm->get_command_count(cm); i++) {
        const char *cmd_name = cm->get_command_name(cm, i);
        get_pal_interface()->uart_send(cmd_name);
        get_pal_interface()->uart_send("\n");
    }
}

// Reboot 命令实现
static void reboot_command(int argc, char *argv[]) {
    get_pal_interface()->uart_send("Rebooting...\n");
    // 模拟重启操作
    // exit(0);
}

// Clear 命令实现
static void clear_command(int argc, char *argv[]) {
    get_pal_interface()->uart_send("\033[H\033[J"); // 清屏 ANSI 转义码
}