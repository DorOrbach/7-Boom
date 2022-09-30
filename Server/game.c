/*This module is the main operator of the game functions.
* It deals with holdong the current status of the game and update it after each move.
* It also checks if the game is over and what is the next correct move to play.
* Remark: we did not explain much about functions which operates exactly as their name suggests.
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "game.h"
#include "send_receive.h"

/*Description:
* this function check if the next correct move is BOOm or not.
* input: current status of the game.
* output: 1=correct move was booom. 0=boom is not required.
*/
int check_if_boom(GameView* game_stat) {
	int last_move = game_stat->last_played_num;
	last_move++;
	int is_boom = last_move % 7;
	if (is_boom == 0) {
		return 1;
	}
	//if got here then need to check if current number contains 7:
	char* move = NULL;
	move = (char*)malloc(sizeof(char) * strlen(game_stat->player_move));
	if (move == NULL) {
		printf("memory allocation fialed, Exiting");
		exit(1);
	}
	sprintf(move, "%d", last_move);
	if (strstr(move, "7") == NULL) {
		return 0;
	}
	return 1;
}

/*Description: check what is the next correct move in the game.
* this function updates the next move field in the game_stst struct.
*/
int check_next_num_to_play(GameView* game) {
	int current = game->last_played_num;
	int next_num = current + 1;
	return next_num;
}

/*the main function of this module:
* input: game_sts - current status of the game.
*		move - the move that was just played.
*		players_index - the players that played.
*/
void make_move(GameView* game_stat, Message *move,int players_index) {
	//check of current move was boom:
	int is_boom = 0;
	if (strcmp(move->parameters, "boom") == 0) {
		//check if a boom should have been said:
		is_boom = check_if_boom(game_stat, move);
		if (is_boom == 1) {
			//boom is correct so update the current move:
			game_stat->last_played_num++;
			return;
		}
		else {//shouldn't have said boom
			game_stat->is_over = 1;
			return;
		}
	}
	else {//received regular number:
		is_boom = check_if_boom(game_stat, move);
		if (is_boom == 1) {
			//no boom was said so game loose:
			game_stat->is_over = 1;
			return;
		}
		//no boom needed so check if said the correct number:
		int new_move = atoi(move->parameters);
		int next_correct_num = check_next_num_to_play(game_stat);
		if (new_move == next_correct_num) {
			game_stat->last_played_num = new_move;
			return;
		}
		game_stat->is_over = 1;
		return;
	}
}

/*Description:
* initiate a struct that holds the current status of the game.
* the game stat is a global variable that each thread updates at is own turn.
*/
GameView* init_game_view_struct() {
	GameView* game = (GameView*)malloc(sizeof(GameView));
	if (game == NULL) {
		printf("memory allocation failed, exiting\n");
		exit(1);
	}
	game->current_turn = 0;
	game->is_over = 0;
	game->last_played_num = 0;
	game->winner = -1;
	game->prev_player = -1;
	return game;
}