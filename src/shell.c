#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "shell.h"
#include "command.h"
#include "history.h"
#include "pal.h"
#include "log.h"
#if defined(ENABLE_FREERTOS) && (ENABLE_FREERTOS == 1)
#include "FreeRTOS.h"
#include "task.h"
#endif

// 键码定义
#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_LEFT 68
#define KEY_RIGHT 67
#define KEY_ENTER 10
#define KEY_BACKSPACE 127
#define KEY_TAB '\t'

// 命令函数声明
static void hello_command(int argc, char *argv[]);
static void list_command(int argc, char *argv[]);
static void reboot_command(int argc, char *argv[]);
static void clear_command(int argc, char *argv[]);
static void log_command(int argc, char *argv[]);
static void ps_command(int argc, char *argv[]);

// 打印带颜色的 Shell Logo 和版本信息
static void print_logo(Shell *self) {
    // 蓝色输出 Logo
    self->pal->uart_send("\033[34m");  // 设置蓝色
    self->pal->uart_send("\n");
    self->pal->uart_send(" ____  _          _ _ \n");
    self->pal->uart_send("/ ___|| |__   ___| | |\n");
    self->pal->uart_send("\\___ \\| '_ \\ / _ \\ | |\n");
    self->pal->uart_send(" ___) | | | |  __/ | |\n");
    self->pal->uart_send("|____/|_| |_|\\___|_|_|\n");
    self->pal->uart_send("\n");

    // 绿色输出版本信息
    char version_info[64];
    snprintf(version_info, sizeof(version_info), "Embedded Shell v%s\n", SHELL_VERSION);
    self->pal->uart_send("\033[32m");  // 设置绿色
    self->pal->uart_send(version_info);

    // 重置颜色
    self->pal->uart_send("\033[0m");
}

// 验证密码函数
static bool verify_password(Shell *self, const char *password) {
    return strcmp(password, DEFAULT_PASSWORD) == 0;
}

// 登录功能：获取并验证用户输入的密码
static bool login(Shell *self) {
    char password_input[32];
    int attempts = 3;

    while (attempts > 0) {
        // 每次新输入时显示提示符
        self->pal->uart_send("Enter password: ");

        int idx = 0;
        while (true) {
            int ch = self->pal->get_char();
            if (ch == KEY_ENTER) {
                password_input[idx] = '\0';
                break;
            } else if (ch == KEY_BACKSPACE && idx > 0) {
                idx--;
                self->pal->uart_send("\b \b");
            } else if (idx < sizeof(password_input) - 1) {
                password_input[idx++] = ch;
                self->pal->uart_send("*"); // 显示掩码
            }
        }

        self->pal->uart_send("\n");

        if (self->verify_password(self, password_input)) {
            self->log_manager->log(LOG_LEVEL_INFO, "Access granted.");
            return true;
        } else {
            attempts--;
            char message[50];
            snprintf(message, sizeof(message), "Incorrect password. %d attempt(s) remaining.", attempts);
            self->log_manager->log(LOG_LEVEL_WARN, message);

            // 提示重新输入
            if (attempts > 0) {
                self->pal->uart_send("Please try again.\n");
            }
        }
    }

    self->log_manager->log(LOG_LEVEL_ERROR, "Access denied. Login failed after 3 attempts.");
    return false;
}


// 初始化 Shell
static void shell_init(Shell *self) {
    self->pal->init();

    // 执行登录过程
    if (!login(self)) {
        // 登录失败，退出程序
        exit(0);
    }

    print_logo(self);

    // 初始化历史记录和命令
    self->history_manager->init(self->history_manager);
    self->command_manager->register_command(self->command_manager, "hello", hello_command);
    self->command_manager->register_command(self->command_manager, "list", list_command);
    self->command_manager->register_command(self->command_manager, "reboot", reboot_command);
    self->command_manager->register_command(self->command_manager, "clear", clear_command);
    self->command_manager->register_command(self->command_manager, "log", log_command);
    self->command_manager->register_command(self->command_manager, "ps", ps_command);

    // 注册别名
    self->command_manager->register_alias(self->command_manager, "ls", "list");
    self->command_manager->register_alias(self->command_manager, "rb", "reboot");

    // 设置日志级别并记录初始化完成日志
    self->log_manager->set_level(LOG_LEVEL_INFO);
    self->log_manager->log(LOG_LEVEL_INFO, "Shell initialized successfully.");
}

// 注册命令
static int shell_register_command(Shell *self, const char *name, CommandFunction func) {
    int result = self->command_manager->register_command(self->command_manager, name, func);
    if (result == COMMAND_SUCCESS) {
        char log_message[64];
        snprintf(log_message, sizeof(log_message), "Registered command: %s", name);
        self->log_manager->log(LOG_LEVEL_INFO, log_message);
    }
    return result;
}

// 自动补全命令
static void autocomplete_command(Shell *self) {
    int match_count = 0;
    const char *last_match = NULL;
    size_t input_length = strlen(self->input_buffer);

    for (int i = 0; i < self->command_manager->get_command_count(self->command_manager); i++) {
        const char *cmd_name = self->command_manager->get_command_name(self->command_manager, i);
        if (strncmp(cmd_name, self->input_buffer, input_length) == 0) {
            match_count++;
            last_match = cmd_name;
            if (match_count > 1) {
                break;
            }
        }
    }

    if (match_count == 1 && last_match) {
        strncpy(self->input_buffer, last_match, INPUT_BUFFER_SIZE - 1);
        self->buffer_length = strlen(self->input_buffer);
        self->pal->uart_send("\r");
        self->pal->uart_send("shell> ");
        self->pal->uart_send(self->input_buffer);
    }
}

// 处理输入命令
static void process_input(Shell *self) {
    if (self->buffer_length > 0) {
        self->input_buffer[self->buffer_length] = '\0';
        self->history_manager->add(self->history_manager, self->input_buffer);

        char log_message[256];
        snprintf(log_message, sizeof(log_message), "Processing command: %s", self->input_buffer);
        self->log_manager->log(LOG_LEVEL_INFO, log_message);

        if (strcmp(self->input_buffer, "exit") == 0) {
            self->log_manager->log(LOG_LEVEL_WARN, "Shell is exiting.");
            exit(0);
        }

        int result = self->command_manager->execute_command(self->command_manager, self->input_buffer);
        if (result != COMMAND_SUCCESS) {
            // 记录具体的错误信息
            switch (result) {
                case COMMAND_ERROR_TABLE_FULL:
                    self->log_manager->log(LOG_LEVEL_ERROR, "Failed to register command: Command table full.");
                    break;
                case ALIAS_ERROR_TABLE_FULL:
                    self->log_manager->log(LOG_LEVEL_ERROR, "Failed to register alias: Alias table full.");
                    break;
                case COMMAND_ERROR_NO_INPUT:
                    self->log_manager->log(LOG_LEVEL_ERROR, "No input provided for command.");
                    break;
                case COMMAND_ERROR_NOT_FOUND:
                    self->log_manager->log(LOG_LEVEL_ERROR, "Command not found.");
                    break;
                default:
                    self->log_manager->log(LOG_LEVEL_ERROR, "Unknown command error.");
                    break;
            }
        }
        
        // 重置缓冲区和游标位置
        self->buffer_length = 0;
        self->cursor_position = 0;
        memset(self->input_buffer, 0, sizeof(self->input_buffer));
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

        case EVENT_KEY_TAB:
            autocomplete_command(self);
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

        case EVENT_KEY_LEFT:
            if (self->cursor_position > 0) {
                self->cursor_position--;
                self->pal->uart_send("\b"); // 移动光标向左
            }
            break;

        case EVENT_KEY_RIGHT:
            if (self->cursor_position < self->buffer_length) {
                self->pal->uart_send((char[]){self->input_buffer[self->cursor_position], '\0'});
                self->cursor_position++;
            }
            break;

        case EVENT_KEY_CHAR:
            if (self->buffer_length < sizeof(self->input_buffer) - 1) {
                // 将字符插入到游标位置，并将缓冲区后续字符后移
                memmove(&self->input_buffer[self->cursor_position + 1],
                        &self->input_buffer[self->cursor_position],
                        self->buffer_length - self->cursor_position);

                self->input_buffer[self->cursor_position] = (char)data; // 插入字符
                self->buffer_length++;
                self->cursor_position++;

                // 重新显示缓冲区内容
                self->pal->uart_send("\r");  // 回到行首
                self->pal->uart_send("shell> ");
                self->pal->uart_send(self->input_buffer);

                // 移动光标到新的位置
                int move_left = self->buffer_length - self->cursor_position;
                for (int i = 0; i < move_left; i++) {
                    self->pal->uart_send("\b");
                }
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
            } else if (ch == KEY_TAB) {
                self->handle_event(self, EVENT_KEY_TAB, 0);
            } else if (ch == 27) { // 方向键序列以 27 开头
                self->pal->get_char(); // 跳过 '['
                ch = self->pal->get_char();
                if (ch == KEY_UP) {
                    self->handle_event(self, EVENT_KEY_UP, 0);
                } else if (ch == KEY_DOWN) {
                    self->handle_event(self, EVENT_KEY_DOWN, 0);
                } else if (ch == KEY_LEFT) {
                    self->handle_event(self, EVENT_KEY_LEFT, 0);
                } else if (ch == KEY_RIGHT) {
                    self->handle_event(self, EVENT_KEY_RIGHT, 0);
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
    shell->log_manager = get_log_manager();  // 获取日志管理器
    shell->init = shell_init;
    shell->verify_password = verify_password;  // 设置验证函数
    shell->loop = shell_loop;
    shell->register_command = shell_register_command;
    shell->handle_event = shell_handle_event;
    shell->buffer_length = 0;
    shell->cursor_position = 0;               // 初始化游标位置

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

// log 命令实现
static void log_command(int argc, char *argv[]) {
    LogManager *log_manager = get_log_manager();

    if (argc == 3 && strcmp(argv[1], "-level") == 0) {
        int level = atoi(argv[2]);

        switch (level) {
            case 0:
                log_manager->set_level(LOG_LEVEL_ERROR);
                log_manager->log(LOG_LEVEL_INFO, "Log level set to ERROR.");
                break;
            case 1:
                log_manager->set_level(LOG_LEVEL_WARN);
                log_manager->log(LOG_LEVEL_INFO, "Log level set to WARN.");
                break;
            case 2:
                log_manager->set_level(LOG_LEVEL_INFO);
                log_manager->log(LOG_LEVEL_INFO, "Log level set to INFO.");
                break;
            default:
                log_manager->log(LOG_LEVEL_ERROR, "Invalid log level. Use 0 for ERROR, 1 for WARN, or 2 for INFO.");
                break;
        }
    } else {
        log_manager->log(LOG_LEVEL_ERROR, "Usage: log -level <0|1|2>");
    }
}

static void ps_command(int argc, char *argv[]) {
    LogManager *log_manager = get_log_manager();

    #if defined(ENABLE_FREERTOS) && (ENABLE_FREERTOS == 1) && \
        defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1) && \
        defined(configUSE_STATS_FORMATTING_FUNCTIONS) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)

        char buffer[1024];          // 用于存储任务信息的缓冲区
        const int refresh_delay = 500000;  // 刷新间隔（500毫秒）
        PalInterface *pal = get_pal_interface();

        log_manager->log(LOG_LEVEL_INFO, "Press 'q' to stop the task list refresh and return to shell.");

        while (true) {
            // 检查用户输入是否为退出键
            if (pal->is_key_pressed() && pal->get_char() == 'q') {
                log_manager->log(LOG_LEVEL_INFO, "\nExiting task list view...");
                break;
            }

            // 清除屏幕并回到顶部位置
            log_manager->log(LOG_LEVEL_INFO, "\033[2J\033[H");

            // 输出标题
            log_manager->log(LOG_LEVEL_INFO, "Task Name\tState\tPriority\tStack\tTask Number");
            log_manager->log(LOG_LEVEL_INFO, "-------------------------------------------------");

            // 获取和显示任务列表
            vTaskList(buffer);
            log_manager->log(LOG_LEVEL_INFO, buffer);

            // 延迟以控制刷新速度
            usleep(refresh_delay);
        }

    #else
        log_manager->log(LOG_LEVEL_ERROR, "Error: FreeRTOS task list feature is disabled.");
        log_manager->log(LOG_LEVEL_ERROR, "Ensure ENABLE_FREERTOS, configUSE_TRACE_FACILITY, and configUSE_STATS_FORMATTING_FUNCTIONS are defined and set to 1.");
    #endif
}


