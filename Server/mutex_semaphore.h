#ifndef MUTEX_SEMAPHORE_H
#define MUTEX_SEMAPHORE_H

#include <windows.h>

HANDLE create_semaphore(int init_num, int max_num);

#endif // !MUTEX_SEMAPHORE_H
