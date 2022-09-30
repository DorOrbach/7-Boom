/*Module description:
* this function deals with setting up the communication between the client and the server:
* - initiate the socket.
* - connect to the server.
* - recieving messages.
* - perform the correct operation regarding each message.
*/
#include "connectClient.h"
#include <stdio.h>
#include "SocketSendRecvTools.h"
#include "clientServerMessages.h"


#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345
#define MAX_LEN 20
#define MAX_MSG 200
#define LOG_LEN 33 
#define LOG_MSG 221
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

char *ip=NULL;
int port=0;
char *name=NULL;
char *file=NULL;
SOCKET m_socket; // comuunication socket

/*Description - group of functions that deals with log file.
* we won't elaborate because the do as their names suggest.
*/
void delete_old_logs(const char* path) {
	FILE* ptr = NULL;
	ptr = fopen(path, "w");
	if (ptr == NULL) {
		printf("Error, can't open log file. exiting\n");
		exit(1);
	}
	fclose(ptr);
}
void print_to_log_file(char* path, char* message) {
	FILE* ptr = NULL;
	ptr = fopen(path, "a");
	if (ptr == NULL) {
		printf("error: could not find the path to log file\n");
		//freeAndExit
	}
	fprintf(ptr, "%s\n", message);
	fclose(ptr);
}

/*Description
* This function exit from the program, closing the socket and realeasing the resources.
* recieve - socket and log file 
*/
void freeAndExit(SOCKET* m_socket, Msg* msg_from_srv, char* log_file_name) {
	if (msg_from_srv != NULL) {
		free(msg_from_srv);
	}
	closesocket(m_socket);
	printf("Server disconnected. Exiting.\n");
	char log[LOG_MSG];
	sprintf(log, "Server disconnected. Exiting.\n");
	print_to_log_file(log_file_name, log);
	WSACleanup();
	exit(1);
}

/*Description
* This function initialize the message struct
* recieve - raw message from user
* retun - message struct include the message and he's params
*/
Msg storeMessage(char* raw_message) {
	Msg* msg = malloc(sizeof(Msg) * strlen(raw_message));
	char msgcpy[MAX_MSG];
	strcpy(msgcpy, raw_message);
	msg->type = malloc(sizeof(char) * strlen(raw_message));
	msg->param1 = malloc(sizeof(char) * strlen(raw_message));
	msg->param2 = malloc(sizeof(char) * strlen(raw_message));
	msg->param3 = malloc(sizeof(char) * strlen(raw_message));
	if (msg == NULL || msg->type == NULL || msg->param1 == NULL || msg->param2 == NULL || msg->param3 == NULL) {
		printf("memory allocation failed. exiting\n");
		exit(1);
	}
	char* message_type = strtok(msgcpy, ":");
	strcpy(msg->type, message_type);
	message_type = strtok(NULL, ";");
	if (message_type != NULL) {
		strcpy(msg->param1, message_type);
	}
	message_type = strtok(NULL, ";");
	if (message_type != NULL) {
		strcpy(msg->param2, message_type);
	}
	message_type = strtok(NULL, "\n");
	if (message_type != NULL) {
		strcpy(msg->param3, message_type);
	}
	return *msg;
}


SOCKET m_socket; // comuunication socket

/*Description
* This function creates the socket and connecting to the server
* recieve - ip, port, user name from the main function
* also openning the client log file
*/
void mainClient(char* server_ip, int server_port, char* user_name) {
	//create log file
	char log_file_name[LOG_LEN];
	sprintf(log_file_name, "Client_log_%s.txt", user_name);
	delete_old_logs(log_file_name);
	ip= server_ip;
	port = server_port;
	name= user_name;
	file= log_file_name;
	Sleep(3000);
	WSADATA wsaData;
	//Call WSAStartup and check for errors.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup()\n");
	}
	//create socket:
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Failed connecting to server on %s:%d. Exiting\n", SERVER_ADDRESS_STR, server_port);
		WSACleanup();
		return;
	}
	int server_port_int = 0;
	SOCKADDR_IN clientService;
	//Create a sockaddr_in object clientService and set values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(server_port); //Setting the port to connect to.

	//connect to server:
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
	{
		//connection failed
		printf("Failed connecting to server on %s:%d\n", SERVER_ADDRESS_STR, server_port);
		//write to log
		char log[LOG_MSG];
		sprintf(log, "Failed connecting to server on %s:%d\n", SERVER_ADDRESS_STR, server_port);
		print_to_log_file(log_file_name, log);
		connectionFailed(server_ip, server_port, user_name, NULL, log_file_name);
	}
	else {
		//connection succeed
		printf("Connected to server on %s:%d\n", SERVER_ADDRESS_STR, server_port);//also to log
		char log[LOG_MSG];
		sprintf(log, "Connected to server on %s:%d\n", SERVER_ADDRESS_STR, server_port);
		print_to_log_file(log_file_name, log);
		newConnectedClient(server_ip, server_port, user_name, &m_socket, log_file_name);		
		return;
	}
}

/*Description:
* After a succssive connection this function recieve the messages from the server
* And sends them to the serverMessegas function to proccesing
*/
void newConnectedClient(char* server_ip, int server_port, char* user_name, SOCKET* m_socket, char* log_file_name) {
	char send_str[MAX_MSG];
	char buff[MAX_MSG];
	sprintf(send_str, "CLIENT_REQUEST:%s", user_name);	
	TransferResult_t send_res = SendString(send_str, *m_socket);
	char log[LOG_MSG];
	sprintf(log, "sent to server - CLIENT_REQUEST: %s\n", user_name);
	print_to_log_file(log_file_name, log);
	if (TRNS_SUCCEEDED != send_res) {
		Sleep(15000);
		TransferResult_t send_res = SendString(send_str, *m_socket);
		sprintf(log, "sent to server - CLIENT_REQUEST: %s\n", user_name);
		if (TRNS_SUCCEEDED != send_res) {
			//connection failed
			printf("Failed connecting to server on %s:%d\n", SERVER_ADDRESS_STR, server_port);//also to log
			//write to log
			char log[LOG_MSG];
			sprintf(log, "Failed connecting to server on %s:%d\n", SERVER_ADDRESS_STR, server_port);
			print_to_log_file(log_file_name, log);
			connectionFailed(server_ip, server_port, user_name, NULL, log_file_name);
		}
	}
	else
		//connection secceded - starting to get messages from server
	{
		while (1)
		{
			char* rec_str = NULL;
			TransferResult_t recv_res = ReceiveString(&rec_str, *m_socket);
			if (TRNS_SUCCEEDED == recv_res) {
				Msg* msg_from_srv = malloc(sizeof(Msg));
				*msg_from_srv = storeMessage(rec_str);
				serverMessages(server_ip, server_port, user_name, m_socket, msg_from_srv, log_file_name);
			}
			else {
				//else server disconnected
				printf("Failed connecting to server on %s:%d\n", server_ip, server_port);
				//add W to log
				char log[LOG_MSG];
				sprintf(log, "Failed connecting to server on %s:%d\n", server_ip, server_port);
				print_to_log_file(log_file_name, log);
				Msg* msg_from_srv = malloc(sizeof(Msg));
				msg_from_srv = NULL;
				connectionFailed(server_ip, server_port, user_name, msg_from_srv, log_file_name);
			}
			

		}
	}
	return;

}