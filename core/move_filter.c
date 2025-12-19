// Filter legal moves from pseudo-moves

// Creates a copy of the pseudo-legal list moves and filters the legal moves
// Improve -> remove the copy to save memory

#include "move_filter.h"

#define TRACE printf("HERE %s:%d\n", __FILE__, __LINE__);


// Unnecessary recalculation of all attackers
bool is_square_attacked(struct Board* board, int square, bool by_white) {
    uint64_t attackers;

    if (by_white) {
        // White pawns attack
        if ((square >= 9) && ((square % 8) != 0) && (board->bitboards[white_pawn] & (1ULL << (square - 9)))) return true;
        if ((square >= 7) && ((square % 8) != 7) && (board->bitboards[white_pawn] & (1ULL << (square - 7)))) return true;

        // Knights and king
        if (knight_attacks[square] & board->bitboards[white_knight]) return true;
        if (king_attacks[square] & board->bitboards[white_king]) return true;

        // Bishop/Queen (diagonal)
        attackers = diagonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[white_bishop] | board->bitboards[white_queen])) return true;

        // Rook/Queen (orthogonal)
        attackers = orthogonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[white_rook] | board->bitboards[white_queen])) return true;
    } else {
        // Black pawns attack
        if ((square <= 54) && ((square % 8) != 0) && (board->bitboards[black_pawn] & (1ULL << (square + 7)))) return true;
        if ((square <= 55) && ((square % 8) != 7) && (board->bitboards[black_pawn] & (1ULL << (square + 9)))) return true;

        // Knights and king
        if (knight_attacks[square] & board->bitboards[black_knight]) return true;
        if (king_attacks[square] & board->bitboards[black_king]) return true;

        // Bishop/Queen (diagonal)
        attackers = diagonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[black_bishop] | board->bitboards[black_queen])) return true;

        // Rook/Queen (orthogonal)
        attackers = orthogonal_attacks(square, board->all_occupied);
        if (attackers & (board->bitboards[black_rook] | board->bitboards[black_queen])) return true;
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


bool check_castling_legality(struct Board* board, uint32_t move) {
    
    if (board->player_turn == white_player) {
        // White kingside castling
        if (((move & 0x3F) == 4) && (((move >> 6) & 0x3F) == 6)) {
            if (is_square_attacked(board, 4, white_player) ||
                is_square_attacked(board, 5, white_player) ||
                is_square_attacked(board, 6, white_player))
                return false;
        }
        // White queenside castling
        else if (((move & 0x3F) == 4) && (((move >> 6) & 0x3F) == 2)) {
            if (is_square_attacked(board, 4, white_player) ||
                is_square_attacked(board, 3, white_player) ||
                is_square_attacked(board, 2, white_player))
                return false;
        }
    } else {
        // Black kingside castling
        if (((move & 0x3F) == 60) && (((move >> 6) & 0x3F) == 62)) {
            if (is_square_attacked(board, 60, black_player) ||
                is_square_attacked(board, 61, black_player) ||
                is_square_attacked(board, 62, black_player)) {
                return false;
            }
        }
        // Black queenside castling
        else if (((move & 0x3F) == 60) && (((move >> 6) & 0x3F) == 58)) {
            if (is_square_attacked(board, 60, black_player) ||
                is_square_attacked(board, 59, black_player) ||
                is_square_attacked(board, 58, black_player)) 
                return false;
        }
    }
    return true;
}


bool is_legal_move(struct Board* board, uint32_t move) {
    struct Board board_copy = *board;

    if (move >> MOVE_CASTLING & 0x1) return check_castling_legality(&board_copy, move);

    int from_square = move & 0x3F;
    int to_square   = (move >> 6) & 0x3F;
    bool is_ep = (move >> EN_PASSANT) & 0x1;
    
    // Changes Player Side 
    apply_move(&board_copy, move);
    int king_square = get_king_square(&board_copy, board_copy.player_turn);

    if (is_square_attacked(&board_copy, king_square, !board_copy.player_turn)) return false;
    if (is_ep && (is_square_attacked(&board_copy, king_square, !board_copy.player_turn))) return false;
   
   return true;
}


struct MoveList generate_legal_moves(struct Board* board) {
    struct MoveList legal_moves;
    legal_moves.count = 0;

    struct MoveList pseudo_moves = generate_board_moves(board);

    for(int move_int = 0; move_int < pseudo_moves.count; move_int++) {
        if (is_legal_move(board, pseudo_moves.moves[move_int])) {
            legal_moves.moves[legal_moves.count++] = pseudo_moves.moves[move_int];
        }
    }
    return legal_moves;
}
