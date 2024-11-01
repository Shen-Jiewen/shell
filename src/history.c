#include <string.h>
#include <stdlib.h>
#include "history.h"

// 单例的历史记录管理器
static HistoryManager history_manager;

// 初始化历史记录管理器
void history_init(HistoryManager* self) {
    self->total_count = 0;
    self->current_index = -1;
    memset(self->history, 0, sizeof(self->history));
}

// 添加命令到历史记录
int history_add(HistoryManager* self, const char *command) {
    if (command == NULL || strlen(command) == 0) {
        return -1; // 无效命令，添加失败
    }

    // 循环写入历史记录
    strncpy(self->history[self->total_count % HISTORY_SIZE], command, COMMAND_LENGTH - 1);
    self->history[self->total_count % HISTORY_SIZE][COMMAND_LENGTH - 1] = '\0'; // 确保以 '\0' 结尾
    self->total_count++;
    self->current_index = self->total_count; // 更新索引指向最新记录
    return 0; // 添加成功
}

// 获取上一个历史记录
const char* history_get_previous(HistoryManager* self) {
    if (self->total_count == 0) {
        return NULL; // 没有历史记录
    }

    // 如果当前索引等于总记录数，初始化为最后一条
    if (self->current_index >= self->total_count) {
        self->current_index = self->total_count - 1;
    } else if (self->current_index > 0) {
        // 非初次访问时，前移索引，指向上一个历史记录
        self->current_index--;
    }

    return self->history[self->current_index % HISTORY_SIZE];
}

// 获取下一个历史记录
const char* history_get_next(HistoryManager* self) {
    if (self->total_count == 0) {
        return NULL; // 没有历史记录
    }

    // 当已到达最新命令（底部）时，将 current_index 重置为 total_count，
    // 这样下一次调用 history_get_previous 时将从最新命令开始
    if (self->current_index >= self->total_count - 1) {
        self->current_index = self->total_count; // 设置为总数，表示在底部
        return ""; // 已经是最新命令，返回空字符串
    }

    // 向前移动一条记录
    self->current_index++;
    return self->history[self->current_index % HISTORY_SIZE];
}

// 获取单例历史管理器的指针
HistoryManager* get_history_manager() {
    // 初始化函数指针
    history_manager.init = history_init;
    history_manager.add = history_add;
    history_manager.get_previous = history_get_previous;
    history_manager.get_next = history_get_next;

    return &history_manager;
}
