/*Module description:
* this module deals with proccessing the messages from the server and acting according to them:
* - checking the message type.
* - printing data to client screen.
* - perform the correct operation regarding each message.
*/

//includes ---------------------------------------------
#include "connectClient.h"
#include "clientServerMessages.h"

extern ip;
extern port;
extern name;
extern file;
extern m_socket;


/*Description 
* This function gets the message struct from the server, checking it's type and execute the operation that needed (if needed)
*/
void serverMessages(char* server_ip, int server_port, char* user_name, SOCKET* m_socket, Msg* msg_from_srv, char* log_file_name) {

	if (STRINGS_ARE_EQUAL("SERVER_APPROVED", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		return;
	}
	if (STRINGS_ARE_EQUAL("SERVER_DENIED", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		printf("Server on %s : %s denied the connection request.", server_ip, server_port);
		connectionFailed(server_ip, server_port, user_name, msg_from_srv, log_file_name);
		return;
	}
	if (STRINGS_ARE_EQUAL("SERVER_MAIN_MENU", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		mainMenu(m_socket, msg_from_srv, log_file_name);
		return;
	}
	if (STRINGS_ARE_EQUAL("GAME_STARTED", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		printf("Game is on!\n");
		return;

	}
	if (STRINGS_ARE_EQUAL("TURN_SWITCH", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		if (STRINGS_ARE_EQUAL(msg_from_srv->param1, user_name)) {
			printf("Your turn!\n");
		}
		else
		{
			printf("%s's player turn!\n", msg_from_srv->param1);
		}
		return;

	}
	if (STRINGS_ARE_EQUAL("SERVER_MOVE_REQUEST", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		playMove(m_socket, msg_from_srv);
		return;

	}
	if (STRINGS_ARE_EQUAL("GAME_ENDED", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		printf("%s won!\n", msg_from_srv->param1);
		return;

	}
	if (STRINGS_ARE_EQUAL("SERVER_NO_OPPONENTS", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		mainMenu(m_socket, msg_from_srv, log_file_name);
		return;

	}
	if (STRINGS_ARE_EQUAL("GAME_VIEW", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		printf("%s move was %s\n", msg_from_srv->param1, msg_from_srv->param2);
		return;

	}
	if (STRINGS_ARE_EQUAL("SERVER_OPPONENT_QUIT", msg_from_srv->type)) {
		//write to log
		char log[LOG_MSG];
		sprintf(log, "received from server - %s\n", msg_from_srv->type);
		print_to_log_file(log_file_name, log);
		printf("Opponent quit.\n");
		mainMenu(m_socket, msg_from_srv, log_file_name);
		return;

	}
	if (STRINGS_ARE_EQUAL("SERVER_SHUTDOWN", msg_from_srv->type)) {
		connectionFailed(server_ip, server_port, user_name, msg_from_srv, log_file_name);
		return;
	}
	return;
}

/*Description
* Showing the main menu screen to client as describe in instruction
*/
void mainMenu(SOCKET* m_socket, Msg* msg_from_srv, char* log_file_name)
{
	// client should present the main menu to the user
	{
		int choise;
		printf("Choose what to do next:\n1. Play against another client\n2. Quit\n");
		char choice[2];
		scanf("%s", &choice);
		choise = atoi(choice);
		if (choise != 1 && choise != 2) {
			//wrong input
			printf("Error: Illegal command");
			mainMenu(m_socket, msg_from_srv, log_file_name);
		}
		else if (choise == 1) {
			//correct input
			TransferResult_t send_res = SendString("CLIENT_VERSUS", *m_socket);
			char log[LOG_MSG];
			sprintf(log, "sent to server - CLIENT_VERSUS\n");
			print_to_log_file(log_file_name, log);
			if (TRNS_SUCCEEDED != send_res) {
				connectionFailed(ip, port, name, msg_from_srv, log_file_name);

			}
		}
		else if (choise == 2) {
			//correct input
			TransferResult_t send_res = SendString("CLIENT_DISCONNECT", *m_socket);
			char log[LOG_MSG];
			sprintf(log, "sent to server - CLIENT_DISCONNECT\n");
			print_to_log_file(log_file_name, log);
			if (TRNS_SUCCEEDED != send_res) {
				connectionFailed(ip, port, name, msg_from_srv, log_file_name);
			}
			else
			{
				freeAndExit(*m_socket, msg_from_srv, log_file_name);
			}
		}
	}
	return;
}
/*Description
* Getting the "move" from the client - boom or number and send it to the server
*/
void playMove(SOCKET* m_socket, Msg* msg_from_srv)
{
	//need to add a check for input
	// client should present the main menu to the user
	{
		char* choise[MAX_MSG] = { '\0' };
		char buff[MAX_MSG];
		int move;
		printf("Enter the next number or boom:\n");
		scanf("%s", &choise);
		if (STRINGS_ARE_EQUAL("boom", choise))
			// play is a boom
		{
			TransferResult_t send_res = SendString("CLIENT_PLAYER_MOVE:boom", *m_socket);
			char log[LOG_MSG];
			sprintf(log, "Sent to server - %s\n", "CLIENT_PLAYER_MOVE:boom");
			print_to_log_file(file, log);
			if (TRNS_SUCCEEDED != send_res) {
				connectionFailed(ip, port, name, msg_from_srv, file);
			}

		}
		else {
			// play is a number
			strcpy(buff, "CLIENT_PLAYER_MOVE:");
			strcat(buff, choise);
			TransferResult_t send_res = SendString(buff, *m_socket);
			char log[LOG_MSG];
			sprintf(log, "Sent to server - CLIENT_PLAYER_MOVE:%s\n", choise);
			print_to_log_file(file, log);
			if (TRNS_SUCCEEDED != send_res) {
				connectionFailed(ip, port, name, msg_from_srv, file);
			}
		}
	}
	return;
}

/*Description
* In case that the connection with the server is lost, asking the client wether to reconnect or exit
*/
void connectionFailed(char* server_ip, int server_port, char* user_name, Msg* msg_from_srv, char* log_file_name) {
	printf("Choose what to do next:\n1. Try to reconnect\n2. Exit\n");
	int choise = 0;
	char choice[2];
	scanf("%s", &choice);
	choise = atoi(choice);
	if (choise == 1)
		//choose to try again
	{
		mainClient(server_ip, server_port, user_name);
	}
	else if (choise == 2) {
		//choose to exit
		freeAndExit(m_socket, msg_from_srv, log_file_name);
	}
	WSACleanup();
	return;
}

