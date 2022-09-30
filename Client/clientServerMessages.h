#ifndef CLIENT_SERVER_MESSAGES_H
#define CLIENT_SERVER_MESSAGES_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#define MAX_LEN 20
#define MAX_MSG 200
#define LOG_MSG 221
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )


void serverMessages(char* server_ip, int server_port, char* user_name, SOCKET* m_socket, Msg* msg_from_srv, char* log_file_name);
void mainMenu(SOCKET* m_socket, Msg* msg_from_srv, char* log_file_name);
void playMove(SOCKET* m_socket, Msg* msg_from_srv);
void connectionFailed(char* server_ip, int server_port, char* user_name, Msg* msg_from_srv, char* log_file_name);

#endif