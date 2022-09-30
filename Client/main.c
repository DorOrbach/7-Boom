/*Authors: Yoav Bruker:206020372 and Dor Orbach:204589790.
* main function of the client program.
*/

//includes ---------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SocketSendRecvTools.h"
#include "connectClient.h"

//defines ----------------------------------------------
#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345
#define MAX_LEN 20
#define MAX_MSG 200
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )


int main(int argc, char* argv[]) {
	char server_ip[MAX_MSG];
	char server_port[MAX_MSG];
	char user_name[MAX_MSG];
	strcpy(server_ip, argv[1]);
	strcpy(server_port, argv[2]);
	strcpy(user_name, argv[3]);
	int port = atoi(server_port);
	mainClient(server_ip, port, user_name);
}