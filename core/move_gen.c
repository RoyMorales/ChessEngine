// Generate move attack for pieces

#include "move_gen.h"
#include <stdio.h>

#define TRACE printf("HERE %s:%d\n", __FILE__, __LINE__);

void generate_king_moves(uint64_t kings, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces) {
    while (kings) {
        int from_square = __builtin_ctzll(kings);
        uint64_t attacks = king_attacks[from_square] & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;               // From square
            move |= (to_square << 6);          // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << CAPTURE);        // Capture flag
            }

            move_list->moves[move_list->count++] = move;

            attacks &= attacks - 1; // Clear the least significant bit
        }
        kings &= kings - 1; // Clear the least significant bit
    }
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

                list->moves[list->count++] = move;
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

                list->moves[list->count++] = move;
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

                list->moves[list->count++] = move;
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

                list->moves[list->count++] = move;
            }
        }
    }
}


void generate_white_pawn_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;  // Clear the least signigicant bit

        int to = from + 8;
        if (to >= 64 || (occupancy & (1ULL << to))) continue;

        // Promotion
        if (to >= 56) {
            int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
            for (int i = 0; i < 4; i++) {
                uint32_t move = 0;
                move |= from;                    // From Square
                move |= (to << 6);               // To Square
                move |= (promo_pieces[i] << 12); // Promotion Flag

                move_list->moves[move_list->count++] = move;
            }
        } else {
            uint32_t move = 0;
            move |= from;                // From Square
            move |= (to << 6);           // To Square
            move |= (1 << PAWN_MOVE);    // Pawn Move Flag

            move_list->moves[move_list->count++] = move;
        } 
    }
}


void generate_black_pawn_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;  // Clear the least signigicant bit

        int to = from - 8;
        if (to < 0 || (occupancy & (1ULL << to))) continue;

        // Promotion
        if (to <= 7) {
            int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
            for (int i = 0; i < 4; i++) {
                uint32_t move = 0;
                move |= from;                      // From Square
                move |= (to << 6);                 // To Square
                move |= (promo_pieces[i] << 12);   // Promotion Flag

                move_list->moves[move_list->count++] = move;
            }
        } else {
            uint32_t move = 0;
            move |= from;                           // From Square
            move |= (to << 6);                      // To Square
            move |= (1 << PAWN_MOVE);               // Pawn move flag

            move_list->moves[move_list->count++] = move;
        }
    }
}


void generate_white_pawn_capture_moves(uint64_t pawns, struct MoveList* move_list, uint64_t opponent_pieces) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        uint64_t captures = white_pawn_attacks[from] & opponent_pieces;

        while (captures) {
            int to = __builtin_ctzll(captures);

            // PROMOTION CAPTURE
            if (to >= 56) {
                int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
                for (int i = 0; i < 4; i++) {
                    uint32_t move = 0;
                    move |= from;                     // From Square
                    move |= (to << 6);                // To Square
                    move |= (1 << CAPTURE);           // Capture Flag
                    move |= (1 << PAWN_MOVE);         // Pawn Move Flag
                    move |= (promo_pieces[i] << 12);  // Promotion Flag

                    move_list->moves[move_list->count++] = move;
                }
            } else {
                uint32_t move = 0;
                move |= from;                         // From Square
                move |= (to << 6);                    // To Square
                move |= (1 << CAPTURE);               // Capture Flag
                move |= (1 << PAWN_MOVE);             // Pawn Move Flag

                move_list->moves[move_list->count++] = move;
            }
            captures &= captures - 1; // Clear the least signigicant bit
        }
    }
}


void generate_black_pawn_capture_moves(uint64_t pawns, struct MoveList* move_list, uint64_t opponent_pieces) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        uint64_t captures = black_pawn_attacks[from] & opponent_pieces;

        while (captures) {
            int to = __builtin_ctzll(captures);

            // PROMOTION CAPTURE
            if (to >= 7) {
                int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
                for (int i = 0; i < 4; i++) {
                    uint32_t move = 0;
                    move |= from;                     // From Square
                    move |= (to << 6);                // To Square
                    move |= (1 << CAPTURE);           // Capture Flag
                    move |= (1 << PAWN_MOVE);         // Pawn Move Flag
                    move |= (promo_pieces[i] << 12);  // Promotion Flag

                    move_list->moves[move_list->count++] = move;
                }
            } else {
                uint32_t move = 0;
                move |= from;                         // From Square
                move |= (to << 6);                    // To Square
                move |= (1 << CAPTURE);               // Capture Flag
                move |= (1 << PAWN_MOVE);             // Pawn Move Flag

                move_list->moves[move_list->count++] = move;
            }
            captures &= captures - 1; // Clear the least signigicant bit
        }
    }
}


void generate_white_pawn_double_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from_square = __builtin_ctzll(pawns);
        int to_square = from_square + 16;

        // Check if the pawn is on the second rank and the path is clear
        if (from_square / 8 == 1 &&
            !(occupancy & (1ULL << (from_square + 8))) &&
            !(occupancy & (1ULL << to_square))) {

            uint32_t move = 0;
            move |= from_square;              // From square
            move |= (to_square << 6);         // To square
            move |= (1 << DOUBLE_PUSH);       // Double push flag
            move |= (1 << PAWN_MOVE);         // Pawn move flag

            move_list->moves[move_list->count++] = move;
        }
        pawns &= pawns - 1; // Clear the least significant bit
    }
}


void generate_black_pawn_double_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from_square = __builtin_ctzll(pawns);
        int to_square = from_square - 16;

        // Check if the pawn is on the seventh rank and the path is clear
        if (from_square / 8 == 6 &&
            !(occupancy & (1ULL << (from_square - 8))) &&
            !(occupancy & (1ULL << to_square))) {

            uint32_t move = 0;
            move |= from_square;              // From square
            move |= (to_square << 6);         // To square
            move |= (1 << DOUBLE_PUSH);       // Double push flag
            move |= (1 << PAWN_MOVE);         // Pawn move flag

            move_list->moves[move_list->count++] = move;
        }
        pawns &= pawns - 1; // Clear the least significant bit
    }
}

void generate_white_pawn_en_passant_moves(uint64_t pawns, struct MoveList* move_list, int en_passant_square) {
    if (en_passant_square == EP_NONE) return;

    int ep = en_passant_square;

    int left_from  = ep - 9;  // white pawn at ep-9 captures ep
    int right_from = ep - 7;  // white pawn at ep-7 captures ep

    // LEFT pawn
    if (left_from >= 0 && (pawns & (1ULL << left_from))) {
        uint32_t move = 0;
        move |= left_from;                   // From square
        move |= (ep << 6);                   // To Square
        move |= (1 << EN_PASSANT);           // En Passant flag
        move |= (1 << CAPTURE);              // Capture flag
        move |= (1 << PAWN_MOVE);            // Pawn move flag

        move_list->moves[move_list->count++] = move;
    }

    // RIGHT pawn
    if (right_from >= 0 && (pawns & (1ULL << right_from))) {
        uint32_t move = 0;
        move |= right_from;                  // From square
        move |= (ep << 6);                   // To Square
        move |= (1 << EN_PASSANT);           // En Passant flag
        move |= (1 << CAPTURE);              // Capture flag
        move |= (1 << PAWN_MOVE);            // Pawn move flag

        move_list->moves[move_list->count++] = move;
    }
}


void generate_black_pawn_en_passant_moves(uint64_t pawns, struct MoveList* move_list, int en_passant_square) {
    if (en_passant_square == EP_NONE) return;

    int ep = en_passant_square;

    int left_from  = ep + 7;
    int right_from = ep + 9;

    // LEFT pawn
    if (left_from < 64 && (pawns & (1ULL << left_from))) {
        uint32_t move = 0;
        move |= left_from;                   // From square
        move |= (ep << 6);                   // To Square
        move |= (1 << EN_PASSANT);           // En Passant flag
        move |= (1 << CAPTURE);              // Capture flag
        move |= (1 << PAWN_MOVE);            // Pawn move flag

        move_list->moves[move_list->count++] = move;
    }

    // RIGHT pawn
    if (right_from < 64 && (pawns & (1ULL << right_from))) {
        uint32_t move = 0;
        move |= right_from;                   // From square
        move |= (ep << 6);                   // To Square
        move |= (1 << EN_PASSANT);           // En Passant flag
        move |= (1 << CAPTURE);              // Capture flag
        move |= (1 << PAWN_MOVE);            // Pawn move flag

        move_list->moves[move_list->count++] = move;
    }
}


void generate_white_pawn_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy, uint64_t opponent_pieces, unsigned char en_passant_square) {
    generate_white_pawn_push_moves(pawns, move_list, occupancy);
    generate_white_pawn_double_push_moves(pawns, move_list, occupancy);
    generate_white_pawn_capture_moves(pawns, move_list, opponent_pieces);
    generate_white_pawn_en_passant_moves(pawns, move_list, en_passant_square);
}


void generate_black_pawn_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy, uint64_t opponent_pieces, unsigned char en_passant_square) {
    generate_black_pawn_push_moves(pawns, move_list, occupancy);
    generate_black_pawn_double_push_moves(pawns, move_list, occupancy);
    generate_black_pawn_capture_moves(pawns, move_list, opponent_pieces);
    generate_black_pawn_en_passant_moves(pawns, move_list, en_passant_square);
}


void generate_knight_moves(uint64_t knights, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces) {
    while (knights) {
        int from_square = __builtin_ctzll(knights);
        uint64_t attacks = knight_attacks[from_square] & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;             // From square
            move |= (to_square << 6);        // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << CAPTURE);      // Capture flag
            }

            move_list->moves[move_list->count++] = move;

            attacks &= attacks - 1; // Clear the least significant bit
        }
        knights &= knights - 1; // Clear the least significant bit
    }
}


void generate_bishop_moves(uint64_t bishops, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces, uint64_t all_occupancy) {
    while (bishops) {
        int from_square = __builtin_ctzll(bishops);
        uint64_t attacks = diagonal_attacks(from_square, all_occupancy) & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;              // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << CAPTURE);       // Capture flag
            }

            move_list->moves[move_list->count++] = move;

            attacks &= attacks - 1; // Clear the least significant bit
        }
        bishops &= bishops - 1; // Clear the least significant bit
    }
}


void generate_rook_moves(uint64_t rooks, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces, uint64_t all_occupancy) {
    while (rooks) {
        int from_square = __builtin_ctzll(rooks);
        uint64_t attacks = orthogonal_attacks(from_square, all_occupancy) & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;              // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << CAPTURE);       // Capture flag
            }

            move_list->moves[move_list->count++] = move;

            attacks &= attacks - 1; // Clear the least significant bit
        }
        rooks &= rooks - 1; // Clear the least significant bit
    }
}


void generate_queen_moves(uint64_t queens, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces, uint64_t all_occupancy) {
    while (queens) {
        int from_square = __builtin_ctzll(queens);
        uint64_t attacks_diag = diagonal_attacks(from_square, all_occupancy);
        uint64_t attacks_orth = orthogonal_attacks(from_square, all_occupancy);
        uint64_t attacks = (attacks_diag | attacks_orth) & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;              // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << CAPTURE);      // Capture flag
            }

            move_list->moves[move_list->count++] = move;

            attacks &= attacks - 1; // Clear the least significant bit
        }
        queens &= queens - 1; // Clear the least significant bit
    }
}


struct MoveList generate_board_moves(struct Board* board) {
    struct MoveList move_list;
    move_list.count = 0;

    if (board->player_turn == white_player) {
        generate_king_moves(board->bitboards[white_king], &move_list, board->white_occupied, board->black_occupied);
        generate_castling_moves(board, &move_list);

        generate_white_pawn_moves(board->bitboards[white_pawn], &move_list, board->all_occupied, board->black_occupied, board->en_passant_square);
        generate_knight_moves(board->bitboards[white_knight], &move_list, board->white_occupied, board->black_occupied);
        generate_bishop_moves(board->bitboards[white_bishop], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
        generate_rook_moves(board->bitboards[white_rook], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
        generate_queen_moves(board->bitboards[white_queen], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
    } else {
        generate_king_moves(board->bitboards[black_king], &move_list, board->black_occupied, board->white_occupied);
        generate_castling_moves(board, &move_list);

        generate_black_pawn_moves(board->bitboards[black_pawn], &move_list, board->all_occupied, board->white_occupied, board->en_passant_square);
        generate_knight_moves(board->bitboards[black_knight], &move_list, board->black_occupied, board->white_occupied);
        generate_bishop_moves(board->bitboards[black_bishop], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
        generate_rook_moves(board->bitboards[black_rook], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
        generate_queen_moves(board->bitboards[black_queen], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
    }
    return move_list;
}
