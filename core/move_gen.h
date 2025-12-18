// Header file for move generation

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <stdint.h>

#include "board.h"
#include "attack.h"

#define MAX_MOVES 256

#define PROMOTION_QUEEN 1
#define PROMOTION_ROOK  2
#define PROMOTION_BISHOP 3
#define PROMOTION_KNIGHT 4

#define NO_CASTLING      0
#define WHITE_KINGSIDE   1
#define WHITE_QUEENSIDE  2
#define BLACK_KINGSIDE   4
#define BLACK_QUEENSIDE  8

#define DOUBLE_PUSH 16
#define CAPTURE 17
#define EN_PASSANT 18
#define MOVE_CASTLING 19
#define PAWN_MOVE 20

// uint32_t move representation:
// Bits 0-5:   From square (0-63)
// Bits 6-11:  To square (0-63)
// Bits 12-15: Promotion piece
// Bits 16-20: Move flags (double push, capture, en passant, castling, pawn move)
// Bits 21-31: Unused -> for future use

struct MoveList {
    uint32_t moves[MAX_MOVES];
    int count;
};

struct MoveList generate_board_moves(struct Board* board);

#endif
