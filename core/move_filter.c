// Filter legal moves from pseudo-moves

// Creates a copy of the pseudo-legal list moves and filters the legal moves
// Improve -> remove the copy to save memory

#include "move_filter.h"

#define TRACE printf("HERE %s:%d\n", __FILE__, __LINE__);


// Unnecessary recalculation of all attackers
bool is_square_attacked(struct Board* board, int square, bool player_side) {
    TRACE
    uint64_t attackers;

    if (player_side) {
        if (white_pawn_attacks[square] & board->bitboards[white_pawn]) return true;
        if (knight_attacks[square] & board->bitboards[white_knight]) return true;
        if (king_attacks[square] & board->bitboards[white_king]) return true;

        attackers = diagonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[white_bishop] | board->bitboards[white_queen]))
            return true;

        attackers = orthogonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[white_rook] | board->bitboards[white_queen]))
            return true;
    }
    else {
        if (black_pawn_attacks[square] & board->bitboards[black_pawn]) return true;
        if (knight_attacks[square] & board->bitboards[black_knight]) return true;
        if (king_attacks[square] & board->bitboards[black_king]) return true;

        attackers = diagonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[black_bishop] | board->bitboards[black_queen]))
            return true;

        attackers = orthogonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[black_rook] | board->bitboards[black_queen]))
            return true;
    }
    return false;
}


int get_king_square(struct Board* board, bool player_colour) {
    uint64_t king_square = player_colour ? board->bitboards[white_king] : board->bitboards[black_king];
    if (!king_square) {
        printf("Error: No king found for player %s\n", player_colour ? "White" : "Black");
        return -1;
    }
    return __builtin_ctzll(king_square);
}


bool is_legal_move(struct Board* board, uint32_t move) {
    TRACE
    struct Board board_copy = *board;

    // Changes Player Side 
    apply_move(&board_copy, move);
    int king_square = get_king_square(&board_copy, board_copy.player_turn);
    return !is_square_attacked(&board_copy, king_square, !board_copy.player_turn);
}


struct MoveList generate_legal_moves(struct Board* board) {
    struct MoveList legal_moves;
    legal_moves.count = 0;

    TRACE

    struct MoveList pseudo_moves = generate_board_moves(board);

    TRACE

    for(int move_int = 0; move_int < pseudo_moves.count; move_int++) {
        if (is_legal_move(board, pseudo_moves.moves[move_int])) {
            legal_moves.moves[legal_moves.count++] = pseudo_moves.moves[move_int];
        }
    }
    return legal_moves;
}
