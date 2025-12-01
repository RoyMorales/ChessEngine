// Applies a move to the board and updates game state accordingly.

#include "move_apply.h"

void apply_move(struct Board* board, uint32_t move) {
    int from_square = move & 0x3F;
    int to_square = (move >> 6) & 0x3F;
    bool is_capture = (move >> 17) & 0x1;
    bool is_double_push = (move >> 16) & 0x1;

    uint64_t from_mask = 1ULL << from_square;
    uint64_t to_mask   = 1ULL << to_square;

    bool player_colour = board->player_turn;

    if (player_colour == white_player) {

        for (int piece_type = 0; piece_type < 12; piece_type += 2) {
            if (board->bitboards[piece_type] & from_mask) {

                board->bitboards[piece_type] &= ~from_mask; 
                board->bitboards[piece_type] |=  to_mask;

                if (is_capture) {
                    for (int opp = 1; opp < 12; opp += 2) {
                        if (board->bitboards[opp] & to_mask) {
                            board->bitboards[opp] &= ~to_mask;
                            break;
                        }
                    }
                }

                // --- AUTO PROMOTION (WHITE) ---
                if (piece_type == white_pawn && to_square >= 56) {
                    board->bitboards[white_pawn] &= ~to_mask;
                    board->bitboards[white_queen] |= to_mask;
                }

                if (piece_type == white_pawn && is_double_push)
                    board->en_passant_square = to_square - 8;
                else
                    board->en_passant_square = EP_NONE;

                break;
            }
        }

    } else {

        for (int piece_type = 1; piece_type < 12; piece_type += 2) {
            if (board->bitboards[piece_type] & from_mask) {

                board->bitboards[piece_type] &= ~from_mask;
                board->bitboards[piece_type] |=  to_mask;

                if (is_capture) {
                    for (int opp = 0; opp < 12; opp += 2) {
                        if (board->bitboards[opp] & to_mask) {
                            board->bitboards[opp] &= ~to_mask;
                            break;
                        }
                    }
                }

                // --- AUTO PROMOTION (BLACK) ---
                if (piece_type == black_pawn && to_square <= 7) {
                    board->bitboards[black_pawn] &= ~to_mask;
                    board->bitboards[black_queen] |= to_mask;
                }

                if (piece_type == black_pawn && is_double_push)
                    board->en_passant_square = to_square + 8;
                else
                    board->en_passant_square = EP_NONE;

                break;
            }
        }
    }
    board->player_turn = !board->player_turn;
}

