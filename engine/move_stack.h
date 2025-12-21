// Header file for move stack

#ifndef MOVE_STACK_H
#define MOVE_STACK_H

#include <stdint.h>

struct MoveInfo {
    uint64_t capture_piece_bitboard;
    int capture_piece_type;

    unsigned char castling_rights;
    unsigned char en_passant_square;
    unsigned char half_turn; 
};


#endif