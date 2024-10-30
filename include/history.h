#ifndef HISTORY_H
#define HISTORY_H

void history_init();
void history_add(const char *command);
const char *history_get_previous();
const char *history_get_next();

#endif // HISTORY_H
