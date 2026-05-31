#include "thread_utils.h"

pthread_mutex_t fs_mutex = PTHREAD_MUTEX_INITIALIZER;

void fs_lock(void)   { pthread_mutex_lock(&fs_mutex); }
void fs_unlock(void) { pthread_mutex_unlock(&fs_mutex); }