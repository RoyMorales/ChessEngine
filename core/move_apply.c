// Applies a move to the board and updates game state accordingly.

#include "move_apply.h"

void apply_move(struct Board* board, uint32_t move) {

    print_move(move);

    int from_square = move & 0x3F;
    int to_square   = (move >> 6) & 0x3F;

    bool is_capture = (move >> CAPTURE) & 0x1;
    bool is_double_push = (move >> DOUBLE_PUSH) & 0x1;
    bool is_pawn_move = (move >> PAWN_MOVE) & 0x1;
    bool is_castling = (move >> MOVE_CASTLING) & 0x1;
    bool is_promotion = ((move >> 12) & 0x0F) != 0;  // NOT SURE!!!!!!
    bool is_en_passant = (move >> EN_PASSANT) & 0x1;

    uint64_t from_mask = 1ULL << from_square;
    uint64_t to_mask   = 1ULL << to_square;

    bool player_colour = board->player_turn;

    // Reset en passant unless set by double pawn push
    // ToDo!!
    board->en_passant_square = EP_NONE;

    if (player_colour == white_player) {
        for (int piece_type = 0; piece_type < 12; piece_type += 2) {
            if (board->bitboards[piece_type] & from_mask) {

                // Handle castling (king moves two squares)
                if (piece_type == white_king) {
                    if (from_square == 4 && to_square == 6) { // White kingside
                        board->bitboards[white_rook] &= ~(1ULL << 7);
                        board->bitboards[white_rook] |= 1ULL << 5;
                    } else if (from_square == 4 && to_square == 2) { // White queenside
                        board->bitboards[white_rook] &= ~(1ULL << 0);
                        board->bitboards[white_rook] |= 1ULL << 3;
                    }
                    board->castling_rights &= ~(CASTLE_WK | CASTLE_WQ);
                }

                // Move the piece
                board->bitboards[piece_type] &= ~from_mask;
                board->bitboards[piece_type] |= to_mask;

                // Handle pawn promotion
                if (piece_type == white_pawn && to_square >= 56) {
                    board->bitboards[white_pawn] &= ~to_mask;
                    board->bitboards[white_queen] |= to_mask;
                }

                // Handle captures
                if (is_capture) {
                    for (int opp = 1; opp < 12; opp += 2) {
                        if (board->bitboards[opp] & to_mask) {
                            board->bitboards[opp] &= ~to_mask;
                            break;
                        }
                    }

                    // En passant capture
                    if (piece_type == white_pawn && to_square == board->en_passant_square) {
                        board->bitboards[black_pawn] &= ~(1ULL << (to_square - 8));
                    }
                }

                // Double pawn push sets en passant
                if (piece_type == white_pawn && is_double_push)
                    board->en_passant_square = to_square - 8;

                // Update castling rights if rook moves
                if (piece_type == white_rook) {
                    if (from_square == 0) board->castling_rights &= ~CASTLE_WQ;
                    if (from_square == 7) board->castling_rights &= ~CASTLE_WK;
                }

                break;
            }
        }
    } else { // black
        for (int piece_type = 1; piece_type < 12; piece_type += 2) {
            if (board->bitboards[piece_type] & from_mask) {

                // Handle castling
                if (piece_type == black_king) {
                    if (from_square == 60 && to_square == 62) { // Black kingside
                        board->bitboards[black_rook] &= ~(1ULL << 63);
                        board->bitboards[black_rook] |= 1ULL << 61;
                    } else if (from_square == 60 && to_square == 58) { // Black queenside
                        board->bitboards[black_rook] &= ~(1ULL << 56);
                        board->bitboards[black_rook] |= 1ULL << 59;
                    }
                    board->castling_rights &= ~(CASTLE_BK | CASTLE_BQ);
                }

                // Move the piece
                board->bitboards[piece_type] &= ~from_mask;
                board->bitboards[piece_type] |= to_mask;

                // Handle pawn promotion
                if (piece_type == black_pawn && to_square <= 7) {
                    board->bitboards[black_pawn] &= ~to_mask;
                    board->bitboards[black_queen] |= to_mask;
                }

                // Handle captures
                if (is_capture) {
                    for (int opp = 0; opp < 12; opp += 2) {
                        if (board->bitboards[opp] & to_mask) {
                            board->bitboards[opp] &= ~to_mask;
                            break;
                        }
                    }

                    // En passant capture
                    if (piece_type == black_pawn && to_square == board->en_passant_square) {
                        board->bitboards[white_pawn] &= ~(1ULL << (to_square + 8));
                    }
                }

                // Double pawn push sets en passant
                if (piece_type == black_pawn && is_double_push)
                    board->en_passant_square = to_square + 8;

                // Update castling rights if rook moves
                if (piece_type == black_rook) {
                    if (from_square == 56) board->castling_rights &= ~CASTLE_BQ;
                    if (from_square == 63) board->castling_rights &= ~CASTLE_BK;
                }

                break;
            }
        }
    }

    // Update occupancy bitboards
    board->white_occupied = 0;
    board->black_occupied = 0;
    for (int i = 0; i < 12; i++) {
        if (i % 2 == 0) board->white_occupied |= board->bitboards[i];
        else            board->black_occupied |= board->bitboards[i];
    }
    board->all_occupied = board->white_occupied | board->black_occupied;

    // Update half-move counter
    if (is_capture || is_pawn_move) {
        board->half_turn = 0;
    } else {
        board->half_turn += 1;
    }

    // Update full-move counter
    if (player_colour == black_player) {
        board->counter_turn += 1;
    }

    // Toggle turn
    board->player_turn = !board->player_turn;
}


