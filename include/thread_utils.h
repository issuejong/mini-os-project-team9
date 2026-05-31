#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

typedef void (*MkdirTaskFunc)(const char *path);

void threaded_mkdir_tasks(char *paths[], int count, MkdirTaskFunc task_func);

#endif
