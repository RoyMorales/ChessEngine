#ifndef EVALUATION_H
#define EVALUATION_H

#include "../core/board.h"
#include "../core/move_gen.h"
#include "../core/move_filter.h"
#include "../core/move_apply.h"

#define INF_SCORE   1000000
#define MATE_SCORE  100000

#define RESULT_WHITE_WINS  1
#define RESULT_BLACK_WINS -1
#define RESULT_DRAW        0
#define RESULT_NONE        2

typedef enum { GAME_PLAYING, GAME_CHECKMATE, GAME_STALEMATE } GameState;
typedef enum { MODE_TWO_PLAYERS, MODE_VS_COMPUTER }           GameMode;

#ifndef PROMOTION_QUEEN
#define PROMOTION_QUEEN  1
#define PROMOTION_ROOK   2
#define PROMOTION_BISHOP 3
#define PROMOTION_KNIGHT 4
#endif

extern double g_time_limit;  // seconds per move, set by UCI

int      evaluation_material(struct Board* board);
int      negamax(struct Board* board, int depth, int alpha, int beta);
uint32_t find_best_move(struct Board* board, int max_depth);

#endif