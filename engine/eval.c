// Evaluation of board

#include <omp.h>
#include "eval.h"

#define INF 1000000
#define MATE 100000

// Piece values (positive = good for white, negative = good for black)
int material[12] = {
    0,    0,    // King
    100, -100,  // Pawn
    320, -320,  // Knight
    330, -330,  // Bishop
    500, -500,  // Rook
    900, -900,  // Queen
};

int evaluation_material(struct Board* board) {
    int score = 0;
    for (int piece = 0; piece < 12; piece++) {
        uint64_t bitboard = board->bitboards[piece];
        while (bitboard) {
            score += material[piece];
            bitboard &= bitboard - 1;
        }
    }
    return score;
}

// Returns the score from the perspective of the side to move
static inline int side_score(struct Board* board) {
    int raw = evaluation_material(board);
    return (board->player_turn == white_player) ? raw : -raw;
}

static inline int king_sq(struct Board* b, bool colour) {
    uint64_t bb = colour ? b->bitboards[black_king] : b->bitboards[white_king];
    return bb ? __builtin_ctzll(bb) : -1;
}

// Is the side that JUST moved leaving their king in check?
static inline bool left_king_in_check(struct Board* b) {
    bool just_moved = b->player_turn ^ 1;  // side that made the move
    int sq = king_sq(b, just_moved);
    if (sq < 0) return true;
    return is_square_attacked(b, sq, b->player_turn);
}

// Sequential negamax — called recursively from depth-1 downward.
// Each thread has its own board copy so there are no data races.
int negamax(struct Board* board, int depth) {
    if (depth == 0) {
        return side_score(board);
    }

    struct MoveList moves = engine_search_moves(board);
    int legal_count = 0;
    int best = -INF;

    for (int i = 0; i < moves.count; i++) {
        struct Board child = *board;         // private copy — no shared state
        apply_move(&child, moves.moves[i]);

        if (left_king_in_check(&child)) continue;  // illegal, skip

        legal_count++;
        int score = -negamax(&child, depth - 1);
        if (score > best) best = score;
    }

    // Terminal node: checkmate or stalemate
    if (legal_count == 0) {
        int ksq = king_sq(board, board->player_turn);
        if (ksq >= 0 && is_square_attacked(board, ksq, !board->player_turn))
            return -MATE + (10 - depth);  // prefer faster mates
        return 0;  // stalemate
    }

    return best;
}

// Parallel root search: each root move is searched by a separate thread.
// Threads never share a Board — each works on its own private copy.
uint32_t find_best_move(struct Board* board, int depth) {
    struct MoveList moves = engine_search_moves(board);

    uint32_t best_move  = 0;
    int      best_score = -INF;

    #pragma omp parallel for schedule(dynamic, 1) shared(best_move, best_score)
    for (int i = 0; i < moves.count; i++) {
        struct Board child = *board;             // thread-private copy
        apply_move(&child, moves.moves[i]);

        if (left_king_in_check(&child)) continue;  // illegal root move

        int score = -negamax(&child, depth - 1);

        #pragma omp critical
        {
            if (score > best_score) {
                best_score = moves.moves[i] ? score : best_score;
                best_score = score;
                best_move  = moves.moves[i];
            }
        }
    }

    return best_move;
}





