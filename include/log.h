#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

// 定义日志级别
typedef enum {
    LOG_LEVEL_NONE,  // 不输出日志
    LOG_LEVEL_ERROR, // 仅输出错误日志
    LOG_LEVEL_WARN,  // 输出警告和错误日志
    LOG_LEVEL_INFO   // 输出所有日志信息
} LogLevel;

// 日志模块结构体
typedef struct {
    LogLevel current_level;                        // 当前日志级别
    void (*set_level)(LogLevel level);             // 设置日志级别
    void (*log)(LogLevel level, const char *message);  // 打印日志信息
    bool (*is_enabled)(LogLevel level);            // 检查当前级别的日志是否启用
} LogManager;

// 获取日志管理器的单例指针
LogManager* get_log_manager();

#endif // LOG_H
