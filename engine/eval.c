// Evaluation of board

#include "eval.h"

#define INF 1000000
#define MATE 100000

// Pieces Values 
// ToDO! -> 
int material[12] = {
    0, 0, //King
    100, -100, //Pawn
    320, -320, //Knight
    330, -330, //Bishop
    500, -500, //Rook
    900, -900, //Queen
};

int evaluation_material(struct Board* board) {
    int score = 0;
    for (int piece = 0; piece < 12; piece++) {
        uint64_t bitboard = board->bitboards[piece];
        while (bitboard) {
            score += material[piece];
            bitboard &= bitboard - 1; // Clear the least significant bit set
        }
    }
    return score;
}


static inline int king_in_check(struct Board* b) {
    int side_just_moved = b->player_turn ^ 1;
    int king_sq = get_king_square(b, side_just_moved);
    return is_square_attacked(b, king_sq, b->player_turn);
}


int negamax(struct Board* board, int depth) {
    if (depth == 0) {
        return evaluation_material(board);
    }

    int legal_moves_count = 0;
    int best_score = -INF;

    struct MoveList moves = engine_search_moves(board);

    for (int i = 0; i < moves.count; i++) {
        uint32_t move = moves.moves[i];

        // Copy board and make move
        struct Board backup = *board;
        make_move(board, move);

        // Illegal move check
        if(king_in_check(board)) {
            *board = backup; // Undo move
            continue; 
        }

        legal_moves_count++;
        int score = -negamax(board, depth - 1);
        *board = backup; // Undo move

        if (score > best_score) {
            best_score = score;
        }

        if (legal_moves_count == 0) {
            if (king_in_check(board)) return -MATE + depth;  // checkmate
            else return 0;                               // stalemate    
        }
    }
    return best_score;
}






