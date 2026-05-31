#include "thread_utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREAD_PATH_MAX 1024

typedef struct {
    char path[THREAD_PATH_MAX];
    MkdirTaskFunc task_func;
} MkdirThreadTask;

static pthread_mutex_t fs_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *mkdir_thread_runner(void *arg)
{
    MkdirThreadTask *task = (MkdirThreadTask *)arg;

    pthread_mutex_lock(&fs_mutex);

    task->task_func(task->path);

    pthread_mutex_unlock(&fs_mutex);

    return NULL;
}

void threaded_mkdir_tasks(char *paths[], int count, MkdirTaskFunc task_func)
{
    if (paths == NULL || count <=0 || task_func == NULL) {
        return;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * count);
    MkdirThreadTask *tasks = malloc(sizeof(MkdirThreadTask) * count);

    if (threads == NULL || tasks == NULL) {
        printf("mkdir: failed to allocate thread resources\n");
        free(threads);
        free(tasks);
        return;
    }

    int created_count = 0;

    for (int i = 0; i < count; i++) {
        strncpy(tasks[i].path, paths[i], THREAD_PATH_MAX - 1);
        tasks[i].path[THREAD_PATH_MAX - 1] = '\0';
        tasks[i].task_func = task_func;

        if (pthread_create(&threads[i], NULL, mkdir_thread_runner, &tasks[i]) != 0) {
            printf("mkdir: failed to create thread for %s\n", paths[i]);
        } else {
            created_count++;
        }
    }

    for (int i = 0; i < created_count; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(tasks);
}
