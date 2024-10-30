#include <string.h>
#include "history.h"

#define HISTORY_SIZE 10
#define COMMAND_LENGTH 128

static char history[HISTORY_SIZE][COMMAND_LENGTH];
static int history_count = 0;
static int current_index = -1;

void history_init() {
    history_count = 0;
    current_index = -1;
    memset(history, 0, sizeof(history));
}

void history_add(const char *command) {
    if (strlen(command) > 0) {
        strncpy(history[history_count % HISTORY_SIZE], command, COMMAND_LENGTH - 1);
        history_count++;
        current_index = history_count;
    }
}

const char *history_get_previous() {
    if (history_count == 0) {
        return NULL;
    }
    if (current_index > 0) {
        current_index--;
    }
    return history[current_index % HISTORY_SIZE];
}

const char *history_get_next() {
    if (history_count == 0) {
        return NULL;
    }
    if (current_index < history_count - 1) {
        current_index++;
    } else {
        return "";
    }
    return history[current_index % HISTORY_SIZE];
}
