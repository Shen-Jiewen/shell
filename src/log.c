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

// 获取日志级别的颜色
static const char* get_color_code(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_INFO:
            return "\033[32m";  // 绿色
        case LOG_LEVEL_WARN:
            return "\033[33m";  // 黄色
        case LOG_LEVEL_ERROR:
            return "\033[31m";  // 红色
        default:
            return "\033[0m";   // 默认颜色
    }
}

// 打印日志信息
static void log_message(LogLevel level, const char *message) {
    if (!log_is_enabled(level)) {
        return;  // 当前日志级别未启用
    }

    // 获取日志前缀和颜色
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

    // 发送带颜色的日志信息到 PAL 层的 UART 或终端
    PalInterface *pal = get_pal_interface();
    pal->uart_send(get_color_code(level)); // 设置颜色
    pal->uart_send(prefix);
    pal->uart_send(message);
    pal->uart_send("\033[0m\n"); // 重置颜色
}

// 获取单例日志管理器
LogManager* get_log_manager() {
    log_manager.set_level = log_set_level;
    log_manager.log = log_message;
    log_manager.is_enabled = log_is_enabled;
    return &log_manager;
}
