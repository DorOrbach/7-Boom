#ifndef GAME_H
#define GAME_H

#include "send_receive.h"

typedef struct GameView {
	int current_turn;
	int last_played_num;
	int is_over;
	int winner;
	char *player_move;
	int prev_player;//for printing the game view
}GameView;

GameView* init_game_view_struct();

void make_move(GameView* game_stat, Message* move,int players_index);


#endif // !GAME_H
