#include <stdio.h>
#include "uart.h"

// 模拟串口初始化
void uart_init() {
    // 在嵌入式设备上，这里将初始化UART
    // 在Linux上可以模拟一个串口初始化过程
    printf("UART Initialized.\n");
}

// UART发送函数
void uart_send(const char *message) {
    // 在嵌入式设备上，这里将使用UART发送数据
    // 在Linux上我们将输出到终端
    printf("%s", message);
}
