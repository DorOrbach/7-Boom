#ifndef SERVER_CONNECT_H
#define SERVER_CONNECT_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUM_OF_WORKER_THREADS 2
#define MAX_PLAYERS 2
#define MAX_NAME_LEN 21 // The instructions say max name is 20 so added 1 more for '\0'
#define OPPONENT_WAIT_TIME 15000
#define PLAYER_1 0
#define PLAYER_2 1
#define EXIT_LEN 5
#define LOG_LEN 33 //generic log file name+max name len.
#define GENERIC_MESSAGE_SEND 18
#define GENERIC_REC_MESSAGE 24
//last 2 defines are not assumptions, they are generic start sizes.

typedef struct Player {
	char name[MAX_NAME_LEN];
	int is_turn;
	SOCKET* user_socket;
	char log_path[LOG_LEN];
}Player;


int create_server(int port);

#endif // !SERVER_CONNECT_H
