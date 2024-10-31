#include "shell.h"

int main() {
    // 创建并初始化 Shell 实例
    Shell* shell = create_shell();

    // 启动 shell 主循环
    shell->loop(shell);

    // 释放 Shell 资源（如果有必要）
    // 一般来说，在嵌入式系统中可能不会真正退出主循环，
    // 但可以在实际系统需要时添加销毁函数来释放资源

    return 0;
}
