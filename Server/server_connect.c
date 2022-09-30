/*Module description:
* this function deals with all communication between the server and the clients:
* - initiate the socket.
* - accepting the new connections.
* - send and receive every message.
* - perform the correct operation regarding each message.
*/


//includes ------------------------------
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "server_connect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "send_receive.h"
#include "mutex_semaphore.h"
#include "game.h"

//Global parameters -------------------
Player players_list[MAX_PLAYERS];
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
SOCKET MainSocket = INVALID_SOCKET;
SOCKET DeniedSocket;
HANDLE Denie_thread=NULL;
HANDLE exit_thread_handle;
HANDLE game_start_semaphore=NULL;
HANDLE turn_semaphore=NULL;
int current_players_num=0;
int num_of_connections = 0;
GameView *game_stat=NULL;
int exit_server = 0;
int socket_stat = 0;
int both_got_end_game_message = 0;

//Declarations --------------------------------------

void exit_cleanup(SOCKET MainSocket);
void client_disconnected(int players_index);

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
void save_logfile_path(int players_index) {
	char path[LOG_LEN];
	sprintf(path, "thread_log_%s.txt", players_list[players_index].name);
	strcpy(players_list[players_index].log_path,path);
}
void print_to_log_file(int players_index, char *message) {
	FILE *ptr = NULL;
	ptr = fopen(players_list[players_index].log_path, "a");
	if (ptr == NULL) {
		printf("error: could not find the path to log file\n");
		//disconnect player:
		client_disconnected(players_index);
		//exit program:
		exit_server = 1;
	}
	fprintf(ptr, "%s\n", message);
	fclose(ptr);
}

//Description:
/*a function that is a thread which disconnect clients, in case the server is full.
*/
DWORD server_denied(SOCKET* denied) {
	int SendRes = 0;
	char denied_message[] = "SERVER_DENIED";
	SendRes = SendString(denied_message, denied);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(denied);
		DeniedSocket = NULL;
		socket_stat = 1;
		return 1;
	}
	closesocket(denied);
	DeniedSocket = NULL;
	socket_stat = 0;
	return 0;
}

/*Description:
* This function is used after a game was ended or after one of the clients has disconnected.
* It takes the global param game_stat and restore it to its original state.
*/
void reset_game() {
	game_stat->current_turn = 0;
	game_stat->is_over = 0;
	game_stat->last_played_num = 0;
	game_stat->prev_player = -1;
	game_stat->winner = -1;
}

//in case the player that is handled by the thread has disconnected,
//this function send a notice to the other player.
void send_disconnect_notice_to_other(int players_index) {
	int other = 0;
	if (players_index == 0) {
		other = 1;
	}
	if (players_list[other].user_socket != NULL) {
		int send_res = SendString("SERVER_OPPONENT_QUIT", players_list[other].user_socket);
		//no need to check if client disconnected because it is the job of the others player thread.
	}
}

//In case a client has disconnected or the connection failed,
//this function closes the socket and the handle to this players thread.
//it also decreases the global param "num_of_players" by 1.
void client_disconnected(int players_index) {
	send_disconnect_notice_to_other(players_index);
	closesocket(players_list[players_index].user_socket);
	current_players_num--;
	reset_game();
	//print to screen and log file:
	printf("Player disconnected. Exiting\n");
	print_to_log_file(players_index, "Player disconnected.Exiting");
	CloseHandle(ThreadHandles[players_index]);
	ThreadHandles[players_index] = NULL;
	return ;
}

void *save_new_username(int players_index, char *rec_str) {
	Message* message = NULL;
	//break the message into its arguments:
	message = break_message_to_params(rec_str);
	//message now contains the username - save it to players struct:
	strcpy(players_list[players_index].name, message->parameters);
	return;
}

int server_no_opponent(int players_index) {
	char send_str[] = "SEREVER_NO_OPPONENT";
	int send_res = 0;
	send_res = SendString(send_str, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		client_disconnected(players_index);
		return -1;
	}
	return 0;
}

int show_main_menu(int players_index) {
	char menu_message[] = "SERVER_MAIN_MENU";
	int send_res, rec_res;
	char* AcceptedStr = NULL;
	send_res = SendString(menu_message, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		printf("Service socket error while writing, closing socket\n");
		client_disconnected(players_index);
		return -1;
	}
	rec_res = ReceiveString(&AcceptedStr, players_list[players_index].user_socket);
	if (rec_res == TRNS_FAILED) {
		printf("Service socket error while writing, closing socket\n");
		client_disconnected(players_index);
		return -1;
	}
	if (strcmp(AcceptedStr, "CLIENT_DISCONNECT") == 0) {
		//player choose to disconnect so close his socket:
		client_disconnected(players_index);
		return 0;
	}
	else {//only other option is CLIENT_VERSUS:
		if (current_players_num < 2) {
			current_players_num++;
		}
		return 1;
	}
}

int send_game_started(int players_index) {
	char send_string[] = "GAME_STARTED";
	int send_res = SendString(send_string, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		client_disconnected(players_index);
		return -1;
	}
	return 0;
}
int send_turn_switch(int players_index) {
	char *turn_switch = NULL;
	turn_switch = (char*)malloc(sizeof(char) * 13 + MAX_NAME_LEN);
	if (turn_switch == NULL) {
		printf("memory allocation failed. exiting");
		exit_server = 1;
		return -1;
	}
	print_to_log_file(players_index, "sent to client - TURN_SWITCH");
	sprintf(turn_switch, "TURN_SWITCH:%s", players_list[game_stat->current_turn].name);
	int send_res = SendString(turn_switch, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		exit_server = 1;
		return -1;
	}
	return 0;
}

//asks the current player to make his move.
char* ask_for_number(int players_index) {
	char ask_number[] = "SERVER_MOVE_REQUEST";
	int send_res = 0;
	send_res = SendString(ask_number, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		client_disconnected(players_index);
		exit(1);
	}
	print_to_log_file(players_index, "sent to client - SERVER_MOVE_REQUEST");
	char *rec_message = NULL;
	int rec_res = ReceiveString(&rec_message, players_list[players_index].user_socket);
	if (rec_res == TRNS_FAILED) {
		client_disconnected(players_index);
		exit(1);
	}
	char *log_message = NULL;
	log_message = (char*)malloc(sizeof(char) * GENERIC_REC_MESSAGE + strlen(rec_message));
	if (log_message == NULL) {
		printf("memory allocation failed. Exiting\n");
		exit_server = 1;
		return;
	}
	sprintf(log_message, "received from client - %s", rec_message);
	print_to_log_file(players_index, log_message);
	return rec_message;
}

int send_game_over(int players_index) {
	char* game_over = NULL;
	int winner = game_stat->winner;
	game_over = (char*)malloc(sizeof(char) * GENERIC_MESSAGE_SEND + strlen(players_list[winner].name));
	if (game_over == NULL) {
		printf("Memory allocation failed. Exiting\n");
		exit_server = 1;
		return -1;
	}
	sprintf(game_over, "GAME_ENDED: %s", players_list[winner].name);
	int send_res = SendString(game_over, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		client_disconnected(players_index);
		return -1;
	}
	both_got_end_game_message++;
	return 0;
}

int send_game_view(int players_index) {
	char* send_string = NULL;
	send_string = (char*)malloc(sizeof(char) * GENERIC_MESSAGE_SEND + strlen(players_list[game_stat->prev_player].name) + strlen(game_stat->player_move));
	if (send_string == NULL) {
		printf("Memory allocation failed. Exiting\n");
		exit_server = 1;
		return -1;
	}
	char* last_move = strstr(game_stat->player_move, ":");
	last_move++;
	sprintf(send_string, "GAME_VIEW: %s;%s;%s", players_list[game_stat->prev_player].name, last_move, "GAME_ON");
	int send_res = SendString(send_string, players_list[players_index].user_socket);
	if (send_res == TRNS_FAILED) {
		client_disconnected(players_index);
		return -1;
	}
	return 0;
}

void switch_turns(int players_index) {
	int next_turn = 0;
	if (players_index == 0) {
		next_turn = 1;
	}
	game_stat->prev_player = game_stat->current_turn;
	game_stat->current_turn = next_turn;
}

void start_game(int players_index) {
	//send player GAME_START massage:
	int send_res = 0;
	send_res=send_game_started(players_index);
	print_to_log_file(players_index, "sent to client - GAME_STARTED");
	if (send_res != 0) {
		return;
	}
	int turn_sent = 0;
	while (game_stat->is_over == 0) {
		//send turn switch:
		if (turn_sent == 0) {
			send_res = send_turn_switch(players_index);
			turn_sent = 1;
		}
		//print_to_log_file(players_index, "sent to client - TURN_SWITCH");
		if (send_res != 0) {
			return;
		}
		//check if it is your turn:
		if (game_stat->current_turn != players_index) {
			continue;
		}
		//close the semaphore for this part and ask for players turn:
		int wait_res = WaitForSingleObject(turn_semaphore, INFINITE);
		turn_sent = 0;
		//start with sending Game View:
		if (game_stat->last_played_num != 0 && game_stat->is_over==0) {
			send_res=send_game_view(players_index);
			print_to_log_file(players_index, "sent to client - GAME_VIEW");
			if (send_res != 0) {
				return;
			}
		}
		
		//ask player for number:
		if (game_stat->is_over == 1){
			send_res = send_game_over(players_index);
			ReleaseSemaphore(turn_semaphore, 1, 0);
			break;
		}
		char *players_move = ask_for_number(players_index);
		game_stat->player_move = (char*)malloc(sizeof(char) * strlen(players_move));
		strcpy(game_stat->player_move, players_move);
		Message* move = break_message_to_params(players_move);
		make_move(game_stat, move,players_index);
		if (game_stat->is_over == 1) {
			int winner = 0;
			if (players_index == 0) {
				winner = 1;
			}
			game_stat->winner = winner;
			ReleaseSemaphore(turn_semaphore, 1, 0);
			//send game over message
			send_res = send_game_over(players_index);
			if (send_res != 0) {
				return;
			}
			
			break;
		}
		switch_turns(players_index);
		//finished playing and updating the game_stat struct so allow second players turn:
		ReleaseSemaphore(turn_semaphore, 1, 0);
	}
	if (players_index == game_stat->winner) {
		send_res = send_game_over(players_index);
		if (send_res != 0) {
			return;
		}
	}
	if (both_got_end_game_message == 2) {
		reset_game();
		both_got_end_game_message = 0;
	}
	print_to_log_file(players_index, "sent to client - SERVER_MAIN_MENU");
	send_res=show_main_menu(players_index);
	if (send_res != 1) {
		return;
	}
	waiting_room(current_players_num, players_index);
}

/*Description:
* the waiting romm is a function that make the player wait until a second player has connected and chose to play.
* it is close by a semaphore which only the second player can release.
* the semaphore "offers" an option to disconnect every 15 sec that there are no other players, by sending the player back to main_menu.
*/
int waiting_room(int num_of_players, int players_index) {
	//first player waits for 15 sec and then main menu appear again:
	if (current_players_num == 1) {
		int send_res = 0;
		int choice = 0;
		int wait_res = WAIT_TIMEOUT;
		while (wait_res == WAIT_TIMEOUT) {
			wait_res=WaitForSingleObject(game_start_semaphore, OPPONENT_WAIT_TIME);
			if (wait_res == WAIT_OBJECT_0) {
				break;
			}
			send_res=server_no_opponent(players_index);
			if (send_res != 0) {
				client_disconnected(players_index);
				return -1;
			}
			print_to_log_file(players_index, "sent to client - SHOW_MAIN_MENU");
			choice = show_main_menu(players_index);
			if (choice == 0 || choice == -1) {
				return 0;
			}
			print_to_log_file(players_index, "received from client - CLIENT_VERSUS");
		}
		//play game:
		start_game(players_index);
	}
	//in this case you are the second player and game can start:
	//release semaphore to enter the first player as well:
	ReleaseSemaphore(game_start_semaphore, 1, 0);
	//enter game:
	start_game(players_index);
}

/*Description:
* This function gets the index of the current new connected player.
* Receive his name and save his name and socket into a players_list.
* show the player the main menu and send him to waiting room if he chose to play Send the player to the main menu
*/
int AcceptNewPlayer(int player_number) {
	char *send_str;
	BOOL Done = FALSE;
	TransferResult_t send_res;
	TransferResult_t rec_res;
	char *Accepted_str = NULL;

	int players_index = player_number;
	//wait for client request message:
	rec_res = ReceiveString(&Accepted_str, players_list[players_index].user_socket);
	if (rec_res == TRNS_FAILED) {
		printf("Player disconnected. Ending comunication\n");
		client_disconnected(players_index);
		return -1;
	}
	else {
		//save username:
		save_new_username(players_index, Accepted_str);
		save_logfile_path(players_index);
		delete_old_logs(players_list[players_index].log_path);
		char *to_log = NULL;
		to_log = (char*)malloc(sizeof(char) * strlen(Accepted_str) + GENERIC_MESSAGE_SEND);
		if (to_log == NULL) {
			printf("memort allocation failed. Exiting\n");
			client_disconnected(players_index);
			exit_server = 1;
			return -1;
		}
		sprintf(to_log, "received from client %s:%s", Accepted_str, players_list[players_index].name);
		print_to_log_file(players_index, to_log);
	}
	//shhow player the main menu and wait for his choice:
	print_to_log_file(players_index, "sent to client-SERVER_MAIN_MENU");
	int players_choice = show_main_menu(players_index);
	if (players_choice == 0 || players_choice==-1) {
		//player disconnected:
		num_of_connections--;
		return 0;
	}
	//players choose to play
	print_to_log_file(players_index, "received from client - CLIENT_VERSUS");
	//current_players_num++;
	int waiting_room_choice=waiting_room(current_players_num, players_index);
	if (waiting_room_choice == 0) {//player chose to disconnect
		return 0;
	}

}

//taken from recitaion:
static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

// A thread that always waits until the server user enter "exit" or "EXIT"
//if the word has been entered then it sends the server to an exit procedure.
DWORD WINAPI wait_for_exit(LPVOID PARAM) {
	char buffer[EXIT_LEN];
	while (exit_server == 0) {
		fgets(buffer, EXIT_LEN, stdin);
		if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "EXIT") == 0) {
			exit_server = 1;
		}
	}
	exit_cleanup(MainSocket);
	
}

//mostly taken from recition.
//init the socket and the semaphores.
//listen to new connections and launches a thread for each connection.
//also launches the thread that waits for exit.
int create_server(int port) {
	//start waiting for exit command:
	exit_thread_handle =CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)wait_for_exit,
		exit_server,
		0,
		NULL
	);
	//init semaphore to lock the game until 2 players arrive:
	game_start_semaphore = create_semaphore(0, 1);
	turn_semaphore = create_semaphore(1, 1);
	//init the game view struct:
	game_stat = init_game_view_struct();
	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKADDR_IN service;

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return 1;
	}

	// Create a socket. 
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		exit(1);
	}
	unsigned long Address;
	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		exit(1);
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR);
	service.sin_port = htons(port);

	int bindRes;
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		exit(1);
	}

	int ListenRes;
	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		exit(1);
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	int Ind;
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++) {
		ThreadHandles[Ind] = NULL;
		players_list[Ind].user_socket = NULL;
	}
	printf("Waiting for a client to connect...\n");
	int num_of_players = 0;
	while (exit_server!=1) {
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			//goto server_cleanup_3;
		}
		printf("Client Connected.\n");
		num_of_connections++;
		Ind = FindFirstUnusedThreadSlot();
		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			DeniedSocket = AcceptSocket;
			Denie_thread = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)server_denied,
				DeniedSocket,
				0,
				NULL
			);
		}
		else
		{
			ThreadInputs[Ind] = AcceptSocket;
			players_list[Ind].user_socket = AcceptSocket;
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)AcceptNewPlayer,
				Ind,
				0,
				NULL
			);
		}

	}
	//WaitForSingleObject(exit_thread_handle, INFINITE);
	CloseHandle(exit_thread_handle);
	//got here so exit==1: need to close all handles and threads and sockets:
	//exit_cleanup(MainSocket);
	//exit(0);
}

//exit procedure.
void exit_cleanup(SOCKET MainSocket) {
	int i = 0;
	char final_string[] = "SERVER_SHUTDOWN";
	int send_res = 0;
	while (i < NUM_OF_WORKER_THREADS) {
		if (players_list[i].user_socket != NULL) {
			send_res = SendString(final_string,players_list[i].user_socket);
			shutdown(players_list[i].user_socket, SD_RECEIVE);
			closesocket(players_list[i].user_socket);
		}
		i++;
	}
	closesocket(MainSocket);

	//free allocated memory:
	CloseHandle(game_start_semaphore);
	CloseHandle(turn_semaphore);
	if (game_stat != NULL) {
		free(game_stat);
	}
	exit(0);
}
