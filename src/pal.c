#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "pal.h"

// POSIX 平台的初始化函数，输出初始化信息
static void posix_init() {
    printf("Platform: POSIX Initialized.\n");
}

// POSIX 平台上从标准输入读取一个字符
// - 使用 termios 修改终端模式以禁用回显和缓冲，读取单个字符
static int posix_get_char() {
    struct termios oldt, newt;
    int ch;

    // 获取当前终端设置并备份
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 设置新终端模式：禁用缓冲（ICANON）和回显（ECHO）
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // 读取单个字符
    ch = getchar();

    // 恢复原始终端设置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// POSIX 平台上的串口发送实现
// - 在终端上打印输出字符串
static void posix_uart_send(const char *str) {
    printf("%s", str);
}

// POSIX 平台的延时函数
// - 使用 usleep 实现毫秒级延时，usleep 接受微秒为单位的输入
static void posix_delay(int ms) {
    usleep(ms * 1000); // 将毫秒转换为微秒进行延时
}

// 静态实例化 PalInterface 结构体，用于存储 POSIX 平台的具体实现
static PalInterface pal = {
    .init = posix_init,
    .get_char = posix_get_char,
    .uart_send = posix_uart_send,
    .delay = posix_delay,
};

// 获取 PalInterface 单例的指针
// - 返回 POSIX 平台的具体实现，方便外部模块调用
PalInterface* get_pal_interface() {
    return &pal;
}
