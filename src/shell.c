#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include "uart.h"
#include "shell.h"
#include "command.h"
#include "history.h"

// 键码定义
#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_ENTER 10
#define KEY_TAB '\t'

// 测试命令的函数
void hello_command(int argc, char *argv[]) {
    if (argc > 1) {
        uart_send("Hello, ");
        uart_send(argv[1]);
        uart_send("!\n");
    } else {
        uart_send("Hello, World!\n");
    }
}

// 列出所有命令
void list_command(int argc, char *argv[]) {
    uart_send("Available commands:\n");
    for (int i = 0; i < get_command_count(); i++) {
        uart_send(" - ");
        uart_send(get_command_name(i));
        uart_send("\n");
    }
}

// 模拟重启命令
void reboot_command(int argc, char *argv[]) {
    uart_send("Rebooting...\n");
}

// 模拟系统状态查询
void status_command(int argc, char *argv[]) {
    uart_send("System Status:\n");
    uart_send(" - CPU Usage: 25\n");
    uart_send(" - Memory Usage: 40\n");
    uart_send(" - Temperature: 45C\n");
}

// 模拟内存检查命令
void memcheck_command(int argc, char *argv[]) {
    uart_send("Memory Check:\n");
    uart_send(" - Total Memory: 64KB\n");
    uart_send(" - Used Memory: 25KB\n");
    uart_send(" - Free Memory: 39KB\n");
}

// 模拟GPIO状态命令
void gpio_command(int argc, char *argv[]) {
    if (argc == 3) {
        if (strcmp(argv[1], "set") == 0) {
            uart_send("Setting GPIO pin ");
            uart_send(argv[2]);
            uart_send(" to HIGH.\n");
        } else if (strcmp(argv[1], "clear") == 0) {
            uart_send("Clearing GPIO pin ");
            uart_send(argv[2]);
            uart_send(" to LOW.\n");
        } else {
            uart_send("Usage: gpio set|clear <pin_number>\n");
        }
    } else {
        uart_send("Usage: gpio set|clear <pin_number>\n");
    }
}

// 读取单个字符，不需要回车
int get_char() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// 自动补全命令
void autocomplete_command(char *input_buffer, int *buffer_length) {
    int command_count = get_command_count();
    for (int i = 0; i < command_count; i++) {
        const char *command_name = get_command_name(i);
        if (strncmp(input_buffer, command_name, *buffer_length) == 0) {
            // 补全命令
            strcpy(input_buffer, command_name);
            *buffer_length = strlen(input_buffer);
            uart_send("\r");
            uart_send("shell> ");
            uart_send(input_buffer);
            break;
        }
    }
}

// Shell主函数
void shell_loop() {
    char input_buffer[128];
    int buffer_length = 0;

    // 初始化历史记录
    history_init();

    // 注册命令
    register_command("hello", hello_command);
    register_command("list", list_command);
    register_command("reboot", reboot_command);
    register_command("status", status_command);
    register_command("memcheck", memcheck_command);
    register_command("gpio", gpio_command);

    // 注册别名
    register_alias("ls", "list");
    register_alias("rb", "reboot");

    uart_send("Embedded Shell Ready.\n");

    while (true) {
        uart_send("shell> ");  // 提示符
        buffer_length = 0;
        memset(input_buffer, 0, sizeof(input_buffer));

        while (true) {
            int ch = get_char();

            if (ch == KEY_ENTER) {
                uart_send("\n");
                if (buffer_length > 0) {
                    input_buffer[buffer_length] = '\0';
                    history_add(input_buffer);
                    if (strcmp(input_buffer, "exit") == 0) {
                        uart_send("Exiting shell...\n");
                        return;
                    }
                    execute_command(input_buffer);  // 执行命令
                }
                break;
            } else if (ch == 127) {  // 处理退格键
                if (buffer_length > 0) {
                    buffer_length--;
                    uart_send("\b \b");
                }
            } else if (ch == 27) {  // 处理方向键（箭头键的序列以27开头）
                get_char();  // 跳过 '[' 字符
                ch = get_char();
                if (ch == KEY_UP) {
                    const char *previous_command = history_get_previous();
                    if (previous_command) {
                        // 清空当前行
                        while (buffer_length > 0) {
                            uart_send("\b \b");
                            buffer_length--;
                        }
                        // 显示历史命令
                        strncpy(input_buffer, previous_command, sizeof(input_buffer) - 1);
                        buffer_length = strlen(input_buffer);
                        uart_send(input_buffer);
                    }
                } else if (ch == KEY_DOWN) {
                    const char *next_command = history_get_next();
                    if (next_command) {
                        // 清空当前行
                        while (buffer_length > 0) {
                            uart_send("\b \b");
                            buffer_length--;
                        }
                        // 显示历史命令
                        strncpy(input_buffer, next_command, sizeof(input_buffer) - 1);
                        buffer_length = strlen(input_buffer);
                        uart_send(input_buffer);
                    }
                }
            } else if (ch == KEY_TAB) {  // 处理Tab键用于命令补全
                autocomplete_command(input_buffer, &buffer_length);
            } else {
                if (buffer_length < sizeof(input_buffer) - 1) {
                    input_buffer[buffer_length++] = ch;
                    uart_send((char[]){ch, '\0'});
                }
            }
        }
    }
}
