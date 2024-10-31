#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 50
#define COMMAND_LENGTH 128

// 历史记录管理器结构体
typedef struct HistoryManager {
    char history[HISTORY_SIZE][COMMAND_LENGTH]; // 存储历史命令
    int total_count;                            // 记录的命令总数
    int current_index;                          // 当前访问的命令索引

    // 初始化历史记录管理器
    void (*init)(struct HistoryManager* self);

    // 添加命令到历史记录
    int (*add)(struct HistoryManager* self, const char *command);

    // 获取上一个历史记录
    const char* (*get_previous)(struct HistoryManager* self);

    // 获取下一个历史记录
    const char* (*get_next)(struct HistoryManager* self);
} HistoryManager;

// 获取历史管理器的单例指针
HistoryManager* get_history_manager();

#endif // HISTORY_H
