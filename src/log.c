#include <stdio.h>
#include <string.h>
#include "log.h"
#include "pal.h"

// 静态全局的日志管理器单例
static LogManager log_manager = { .current_level = LOG_LEVEL_INFO };

// 设置日志级别
static void log_set_level(LogLevel level) {
    log_manager.current_level = level;
}

// 检查日志级别是否启用
static bool log_is_enabled(LogLevel level) {
    return level <= log_manager.current_level;
}

// 打印日志信息
static void log_message(LogLevel level, const char *message) {
    if (!log_is_enabled(level)) {
        return;  // 当前日志级别未启用
    }

    // 获取日志前缀
    const char *prefix;
    switch (level) {
        case LOG_LEVEL_INFO:
            prefix = "[INFO] ";
            break;
        case LOG_LEVEL_WARN:
            prefix = "[WARN] ";
            break;
        case LOG_LEVEL_ERROR:
            prefix = "[ERROR]";
            break;
        default:
            prefix = "[UNKNOWN]";
    }

    // 发送日志信息到 PAL 层的 UART 或终端
    PalInterface *pal = get_pal_interface();
    pal->uart_send(prefix);
    pal->uart_send(message);
    pal->uart_send("\n");
}

// 获取单例日志管理器
LogManager* get_log_manager() {
    log_manager.set_level = log_set_level;
    log_manager.log = log_message;
    log_manager.is_enabled = log_is_enabled;
    return &log_manager;
}
