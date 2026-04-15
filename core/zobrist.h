// Zobrist hashing for board positions

#pragma once
#include <stdint.h>
#include "board.h"

extern uint64_t zob_piece[12][64];
extern uint64_t zob_castling[16];
extern uint64_t zob_ep[65];   // 0–63 files + 64 = none
extern uint64_t zob_side;

uint64_t board_hash(const struct Board* board);
void zobrist_init(void);