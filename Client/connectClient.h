#ifndef CONNECT_CLIENT_H
#define CONNECT_CLIENT_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <string.h>
#include "SocketSendRecvTools.h"


typedef struct Msg {
	char* type;
	char* param1;
	char* param2;
	char* param3;
}Msg;

void mainClient(char* server_ip, int server_port, char* user_name);
void newConnectedClient(char* server_ip, int server_port, char* user_name, SOCKET* m_socket, char* log_file_name);
void delete_old_logs(const char* path);
void print_to_log_file(char* path, char* message);
void freeAndExit(SOCKET* m_socket, Msg* msg_from_srv, char* log_file_name);
Msg storeMessage(char* raw_message);

#endif