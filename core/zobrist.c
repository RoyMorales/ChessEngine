// Zobrist hashing for board positions

#include "zobrist.h"
#include <stdlib.h>
#include <time.h>

uint64_t zob_piece[12][64];
uint64_t zob_castling[16];
uint64_t zob_ep[65];
uint64_t zob_side;

static uint64_t rand_u64(void) {
    uint64_t x = (uint64_t)rand();
    x ^= (uint64_t)rand() << 15;
    x ^= (uint64_t)rand() << 30;
    x ^= (uint64_t)rand() << 45;
    return x;
}

void zobrist_init(void) {
    srand((unsigned)time(NULL));

    for (int p = 0; p < 12; p++) {
        for (int sq = 0; sq < 64; sq++) {
            zob_piece[p][sq] = rand_u64();
        }
    }

    for (int i = 0; i < 16; i++) {
        zob_castling[i] = rand_u64();
    }

    for (int i = 0; i < 65; i++) {
        zob_ep[i] = rand_u64();
    }

    zob_side = rand_u64();
}

uint64_t board_hash(const struct Board* board) {
    uint64_t hash = 0;

    for (int piece = 0; piece < 12; piece++) {
        uint64_t bb = board->bitboards[piece];

        while (bb) {
            int sq = __builtin_ctzll(bb);
            hash ^= zob_piece[piece][sq];
            bb &= bb - 1;
        }
    }

    hash ^= zob_castling[board->castling_rights & 0x0F];

    int ep_index = (board->en_passant_square != 255)
        ? board->en_passant_square
        : 64;

    hash ^= zob_ep[ep_index];

    if (board->player_turn) {
        hash ^= zob_side;
    }

    return hash;
}

