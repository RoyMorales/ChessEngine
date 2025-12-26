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

    //int from_square = move & 0x3F;
    //int to_square   = (move >> 6) & 0x3F;
    bool is_ep = (move >> EN_PASSANT) & 0x1;
    
    // Changes Player Side 
    apply_move(&board_copy, move);
    int king_square = get_king_square(&board_copy, board_copy.player_turn);

    if (is_square_attacked(&board_copy, king_square, !board_copy.player_turn)) return false;
    if (is_ep && (is_square_attacked(&board_copy, king_square, !board_copy.player_turn))) return false;
   
   return true;
}


void generate_castling_moves(struct Board* board, struct MoveList* list) {
    uint64_t occ = board->all_occupied;

    if (board->player_turn == white_player) {
        // King must be on E1
        if (!(board->bitboards[white_king] & (1ULL << 4))) return;

        // White kingside castling: E1 -> G1
        if (board->castling_rights & WHITE_KINGSIDE) {
            bool empty_f1 = !(occ & (1ULL << 5));
            bool empty_g1 = !(occ & (1ULL << 6));
            if (empty_f1 && empty_g1) {
                uint32_t move = 0;
                move |= 4;                      // From square
                move |=(6 << 6);               // To square
                move |= (1 << MOVE_CASTLING);   // Castling flag

                if (check_castling_legality(board, move)) {
                    list->moves[list->count++] = move;
                }
            }
        }

        // White queenside castling: E1 -> C1
        if (board->castling_rights & WHITE_QUEENSIDE) {
            bool empty_d1 = !(occ & (1ULL << 3));
            bool empty_c1 = !(occ & (1ULL << 2));
            bool empty_b1 = !(occ & (1ULL << 1));
            if (empty_d1 && empty_c1 && empty_b1) {
                uint32_t move = 0;
                move |= 4;                      // From square
                move |= (2 << 6);               // To square
                move |= (1 << MOVE_CASTLING);   // Castling flag

                if (check_castling_legality(board, move)) {
                    list->moves[list->count++] = move;
                }
            }
        }

    } else { // black
        // King must be on E8
        if (!(board->bitboards[black_king] & (1ULL << 60))) return;

        // Black kingside castling: E8 -> G8
        if (board->castling_rights & BLACK_KINGSIDE) {
            bool empty_f8 = !(occ & (1ULL << 61));
            bool empty_g8 = !(occ & (1ULL << 62));
            if (empty_f8 && empty_g8) {
                uint32_t move = 0;
                move |= 60;                     // From square
                move |= (62 << 6);              // To square
                move |= (1 << MOVE_CASTLING);   // Castling flag

                if (check_castling_legality(board, move)) {
                    list->moves[list->count++] = move;
                }
            }
        }

        // Black queenside castling: E8 -> C8
        if (board->castling_rights & BLACK_QUEENSIDE) {
            bool empty_d8 = !(occ & (1ULL << 59));
            bool empty_c8 = !(occ & (1ULL << 58));
            bool empty_b8 = !(occ & (1ULL << 57));
            if (empty_d8 && empty_c8 && empty_b8) {
                uint32_t move = 0;
                move |= 60;                     // From square
                move |= (58 << 6);              // To square
                move |= (1 << MOVE_CASTLING);   // Castling flag

                if (check_castling_legality(board, move)) {
                    list->moves[list->count++] = move;
                }
            }
        }
    }
}

struct MoveList generate_legal_moves(struct Board* board) {
    struct MoveList legal_moves;
    legal_moves.count = 0;

    struct MoveList pseudo_moves = generate_board_moves(board);
    generate_castling_moves(board, &pseudo_moves);

    for(int move_int = 0; move_int < pseudo_moves.count; move_int++) {
        if (is_legal_move(board, pseudo_moves.moves[move_int])) {
            legal_moves.moves[legal_moves.count++] = pseudo_moves.moves[move_int];
        }
    }
    return legal_moves;
}


struct MoveList engine_search_moves(struct Board* board) {
    struct MoveList pseudo_moves = generate_board_moves(board);
    generate_castling_moves(board, &pseudo_moves);
    return pseudo_moves;
}
