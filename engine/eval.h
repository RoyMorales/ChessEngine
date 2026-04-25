// Header File for evaluation module

#ifndef EVALUATION_H
#define EVALUATION_H

#include "../core/board.h"
#include "../core/move_gen.h"
#include "../core/move_filter.h"
#include "../core/move_apply.h"

// Score constants
#define INF_SCORE   1000000
#define MATE_SCORE  100000

// Game result values (white's perspective — used by GUI and future engine)
#define RESULT_WHITE_WINS  1
#define RESULT_BLACK_WINS -1
#define RESULT_DRAW        0
#define RESULT_NONE        2

// Game mode
typedef enum {
    MODE_TWO_PLAYERS,
    MODE_VS_COMPUTER,
} GameMode;

// Promotion piece type constants (shared with move_gen.h)
#ifndef PROMOTION_QUEEN
#define PROMOTION_QUEEN  1
#define PROMOTION_ROOK   2
#define PROMOTION_BISHOP 3
#define PROMOTION_KNIGHT 4
#endif

int      evaluation_material(struct Board* board);
int      negamax(struct Board* board, int depth, int alpha, int beta);
uint32_t find_best_move(struct Board* board, int depth);

#endif