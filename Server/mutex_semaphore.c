#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "mutex_semaphore.h"
#include <windows.h>

//extern game_start_semaphore;

HANDLE create_semaphore(int init_num, int max_num) {
	HANDLE semaphore = NULL;
	semaphore=CreateSemaphore(NULL, init_num,max_num, NULL);
	if (NULL == semaphore) {
		printf("Semaphore allocation failed\n");
		exit(1);
	}
	return semaphore;
}