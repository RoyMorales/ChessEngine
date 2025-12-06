// Header file for move application functions

#ifndef MOVE_APPLY_H
#define MOVE_APPLY_H

#include "board.h"
#include "movegen.h"


#define DOUBLE_PUSH 16
#define CAPTURE 17
#define EN_PASSANT 18
#define MOVE_CASTLING 19
#define PAWN_MOVE 20


void apply_move(struct Board* board, uint32_t move);

#endif 