// Applies a move to the board and updates game state accordingly.

#include "move_apply.h"

void apply_move(struct Board* board, uint32_t move) {
    int from_square = move & 0x3F;               // Bits 0-5
    int to_square = (move >> 6) & 0x3F;         // Bits 6-11
    bool is_capture = (move >> 17) & 0x1;       // Bit 17
    bool is_double_push = (move >> 16) & 0x1;   // Bit 16

    uint64_t from_mask = 1ULL << from_square;
    uint64_t to_mask = 1ULL << to_square;

    bool player_colour = board->player_turn;

    if (player_colour == white_player) {
        // Identify which piece is moving
        for (int piece_type = 0; piece_type < 12; piece_type += 2) { // White pieces are even indexed
            if (board->bitboards[piece_type] & from_mask) {
                // Move the piece
                board->bitboards[piece_type] &= ~from_mask; // Remove from original square
                board->bitboards[piece_type] |= to_mask;    // Place on destination square

                // Handle captures
                if (is_capture) {
                    for (int opp_piece_type = 1; opp_piece_type < 12; opp_piece_type += 2) { // Black pieces are odd indexed
                        if (board->bitboards[opp_piece_type] & to_mask) {
                            board->bitboards[opp_piece_type] &= ~to_mask; // Remove captured piece
                            break;
                        }
                    }
                }

                // Handle double pawn push for en passant
                if (piece_type == white_pawn && is_double_push) {
                    board->en_passant_square = to_square - 8; // Set en passant square
                } else {
                    board->en_passant_square = EP_NONE; // Reset en passant square
                }

                break;
            }
        }
    } else {
        // Identify which piece is moving
        for (int piece_type = 1; piece_type < 12; piece_type += 2) { // Black pieces are odd indexed
            if (board->bitboards[piece_type] & from_mask) {
                // Move the piece
                board->bitboards[piece_type] &= ~from_mask; // Remove from original square
                board->bitboards[piece_type] |= to_mask;    // Place on destination square

                // Handle captures
                if (is_capture) {
                    for (int opp_piece_type = 0; opp_piece_type < 12; opp_piece_type += 2) { // White pieces are even indexed
                        if (board->bitboards[opp_piece_type] & to_mask) {
                            board->bitboards[opp_piece_type] &= ~to_mask; // Remove captured piece
                            break;
                        }
                    }
                }

                // Handle double pawn push for en passant
                if (piece_type == black_pawn && is_double_push) {
                    board->en_passant_square = to_square + 8; // Set en passant square
                } else {
                    board->en_passant_square = EP_NONE; // Reset en passant square
                }
                break;
            }
        }

    }
    // Toggle player turn
    board->player_turn = !board->player_turn;

}
