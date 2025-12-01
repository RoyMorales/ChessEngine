// Header file for move generation

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <stdint.h>

#define MAX_MOVES 256

#define PROMOTION_NONE 0
#define PROMOTION_QUEEN 1
#define PROMOTION_ROOK  2
#define PROMOTION_BISHOP 3
#define PROMOTION_KNIGHT 4

// uint32_t move representation:
// Bits 0-5:   From square (0-63)
// Bits 6-11:  To square (0-63)
// Bits 12-15: Promotion piece (0=knight, 1=bishop, 2=rook, 3=queen)
// Bits 16-19: Move flags (double push, capture, en passant, castling)
// Bits 20-31: Unused -> for future use

struct MoveList {
    uint32_t moves[MAX_MOVES];
    int count;
};

struct MoveList genrate_board_moves(struct Board* board);

#endif


