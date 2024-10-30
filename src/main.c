#include "uart.h"
#include "shell.h"

int main() {
    uart_init();  // 初始化UART
    shell_loop(); // 启动shell
    return 0;
}
