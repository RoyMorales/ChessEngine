// Evaluation and search

#include <omp.h>
#include <string.h>
#include "eval.h"

#define INF  1000000
#define MATE 100000

// ---------------------------------------------------------------------------
// Material values (indexed by piece enum: 0=wK,1=bK,2=wP,3=bP, ...)
// Positive = good for white, negative = good for black
// ---------------------------------------------------------------------------
static const int MATERIAL[12] = {
    0,    0,      // King
    100, -100,    // Pawn
    320, -320,    // Knight
    330, -330,    // Bishop
    500, -500,    // Rook
    900, -900,    // Queen
};

// ---------------------------------------------------------------------------
// Piece-square tables (white's perspective, a1=index 0, h8=index 63)
// Black mirrors vertically: black_pst[sq] = white_pst[sq ^ 56]
// ---------------------------------------------------------------------------
static const int PST_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10,-20,-20, 10, 10,  5,
     5, -5,-10,  0,  0,-10, -5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
     0,  0,  0,  0,  0,  0,  0,  0,
};
static const int PST_KNIGHT[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -30,  5, 10, 15, 15, 10,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30,
   -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 10, 15, 15, 10,  0,-30,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50,
};
static const int PST_BISHOP[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  5,  0,  0,  0,  0,  5,-10,
   -10, 10, 10, 10, 10, 10, 10,-10,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0,  5, 10, 10,  5,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -20,-10,-10,-10,-10,-10,-10,-20,
};
static const int PST_ROOK[64] = {
     0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5, 10, 10, 10, 10, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0,
};
static const int PST_QUEEN[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -10,  5,  5,  5,  5,  5,  0,-10,
     0,  0,  5,  5,  5,  5,  0, -5,
    -5,  0,  5,  5,  5,  5,  0, -5,
   -10,  0,  5,  5,  5,  5,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20,
};
static const int PST_KING_MG[64] = {
    20, 30, 10,  0,  0, 10, 30, 20,
    20, 20,  0,  0,  0,  0, 20, 20,
   -10,-20,-20,-20,-20,-20,-20,-10,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
};

// ---------------------------------------------------------------------------
// Evaluation
// ---------------------------------------------------------------------------
int evaluation_material(struct Board* board) {
    int score = 0;
    for (int piece = 0; piece < 12; piece++) {
        uint64_t bb = board->bitboards[piece];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;

            int val = MATERIAL[piece];
            // PST bonus (white pieces use sq directly, black mirror vertically)
            int pst_sq = (piece & 1) ? (sq ^ 56) : sq;
            int pst = 0;
            switch (piece >> 1) {   // piece type regardless of colour
                case 1: pst = PST_PAWN[pst_sq];   break;
                case 2: pst = PST_KNIGHT[pst_sq];  break;
                case 3: pst = PST_BISHOP[pst_sq];  break;
                case 4: pst = PST_ROOK[pst_sq];    break;
                case 5: pst = PST_QUEEN[pst_sq];   break;
                case 0: pst = PST_KING_MG[pst_sq]; break;
            }
            // Black PST bonus is subtracted (black wants negative score)
            score += (piece & 1) ? -(val + pst) : (val + pst);
        }
    }
    return score;
}

// Score from the perspective of the side to move (negamax convention)
static inline int side_score(struct Board* board) {
    int raw = evaluation_material(board);
    return (board->player_turn == white_player) ? raw : -raw;
}

// ---------------------------------------------------------------------------
// Move ordering — score moves so captures and promotions come first.
// Higher score = searched earlier = more alpha-beta cutoffs.
// ---------------------------------------------------------------------------
static int move_score(uint32_t move) {
    int score = 0;
    if ((move >> CAPTURE) & 1)     score += 10000;
    if ((move >> EN_PASSANT) & 1)  score += 10000;
    if (((move >> 12) & 0xF) == PROMOTION_QUEEN) score += 9000;
    if ((move >> MOVE_CASTLING) & 1) score += 100;
    return score;
}

static void sort_moves(struct MoveList* moves) {
    // Insertion sort — move lists are short (~30 moves), so this is fine
    for (int i = 1; i < moves->count; i++) {
        uint32_t key = moves->moves[i];
        int key_score = move_score(key);
        int j = i - 1;
        while (j >= 0 && move_score(moves->moves[j]) < key_score) {
            moves->moves[j + 1] = moves->moves[j];
            j--;
        }
        moves->moves[j + 1] = key;
    }
}

// ---------------------------------------------------------------------------
// Negamax with alpha-beta pruning
// ---------------------------------------------------------------------------
int negamax(struct Board* board, int depth, int alpha, int beta) {
    if (depth == 0) return side_score(board);

    struct MoveList moves = generate_legal_moves(board);

    // Terminal node
    if (moves.count == 0) {
        int ksq = get_king_square(board, board->player_turn);
        if (ksq >= 0 && is_square_attacked(board, ksq, !board->player_turn))
            return -MATE + (10 - depth);  // checkmate — prefer faster mates
        return 0;  // stalemate
    }

    sort_moves(&moves);

    for (int i = 0; i < moves.count; i++) {
        struct Board child = *board;
        apply_move(&child, moves.moves[i]);

        int score = -negamax(&child, depth - 1, -beta, -alpha);

        if (score >= beta) return beta;   // beta cutoff
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// ---------------------------------------------------------------------------
// Parallel root search
// ---------------------------------------------------------------------------
uint32_t find_best_move(struct Board* board, int depth) {
    struct MoveList moves = generate_legal_moves(board);
    if (moves.count == 0) return 0;

    sort_moves(&moves);

    uint32_t best_move  = moves.moves[0];
    int      best_score = -INF;

    #pragma omp parallel for schedule(dynamic, 1) shared(best_move, best_score)
    for (int i = 0; i < moves.count; i++) {
        struct Board child = *board;
        apply_move(&child, moves.moves[i]);

        int score = -negamax(&child, depth - 1, -INF, INF);

        #pragma omp critical
        {
            if (score > best_score) {
                best_score = score;
                best_move  = moves.moves[i];
            }
        }
    }

    return best_move;
}