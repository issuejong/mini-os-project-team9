#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <pthread.h>

extern pthread_mutex_t fs_mutex;

void fs_lock(void);
void fs_unlock(void);

#endif