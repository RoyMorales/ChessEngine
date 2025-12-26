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
            if (to <= 7) {
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

void generate_white_pawn_en_passant_moves(uint64_t pawns, struct MoveList* move_list, int ep_square) {
    if (ep_square == EP_NONE) return;

    int ep_file = ep_square % 8;
    int ep_rank = ep_square / 8;

    if (ep_rank != 5) return; // White can only capture en passant on rank 6
    
    // Left capture: pawn on file-1, rank-1
    if (ep_file > 0) {
        int from_square = ep_square - 9; // rank-1, file-1
        if (pawns & (1ULL << from_square)) {
            uint32_t move = 0;
            move |= from_square;
            move |= (ep_square << 6);
            move |= (1 << EN_PASSANT);
            move |= (1 << CAPTURE);
            move |= (1 << PAWN_MOVE);
            move_list->moves[move_list->count++] = move;
        }
    }

    // Right capture: pawn on file+1, rank-1
    if (ep_file < 7) {
        int from_square = ep_square - 7; // rank-1, file+1
        if (pawns & (1ULL << from_square)) {
            uint32_t move = 0;
            move |= from_square;
            move |= (ep_square << 6);
            move |= (1 << EN_PASSANT);
            move |= (1 << CAPTURE);
            move |= (1 << PAWN_MOVE);
            move_list->moves[move_list->count++] = move;
        }
    }
}


void generate_black_pawn_en_passant_moves(uint64_t pawns, struct MoveList* move_list, int ep_square) {
    if (ep_square == EP_NONE) return;

    int ep_file = ep_square % 8;
    int ep_rank = ep_square / 8;

    if (ep_rank != 2) return; // Black can only capture en passant on rank 3

    // Left capture: pawn on file-1, rank-1
    if (ep_file > 0) {
        int from_square = ep_square + 7; // rank-1, file-1
        if (pawns & (1ULL << from_square)) {
            uint32_t move = 0;
            move |= from_square;
            move |= (ep_square << 6);
            move |= (1 << EN_PASSANT);
            move |= (1 << CAPTURE);
            move |= (1 << PAWN_MOVE);
            move_list->moves[move_list->count++] = move;
        }
    }

    // Right capture: pawn on file+1, rank-1
    if (ep_file < 7) {
        int from_square = ep_square + 9; // rank-1, file+1
        if (pawns & (1ULL << from_square)) {
            uint32_t move = 0;
            move |= from_square;
            move |= (ep_square << 6);
            move |= (1 << EN_PASSANT);
            move |= (1 << CAPTURE);
            move |= (1 << PAWN_MOVE);
            move_list->moves[move_list->count++] = move;
        }
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
        generate_white_pawn_moves(board->bitboards[white_pawn], &move_list, board->all_occupied, board->black_occupied, board->en_passant_square);
        generate_knight_moves(board->bitboards[white_knight], &move_list, board->white_occupied, board->black_occupied);
        generate_bishop_moves(board->bitboards[white_bishop], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
        generate_rook_moves(board->bitboards[white_rook], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
        generate_queen_moves(board->bitboards[white_queen], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
    } else {
        generate_king_moves(board->bitboards[black_king], &move_list, board->black_occupied, board->white_occupied);
        generate_black_pawn_moves(board->bitboards[black_pawn], &move_list, board->all_occupied, board->white_occupied, board->en_passant_square);
        generate_knight_moves(board->bitboards[black_knight], &move_list, board->black_occupied, board->white_occupied);
        generate_bishop_moves(board->bitboards[black_bishop], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
        generate_rook_moves(board->bitboards[black_rook], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
        generate_queen_moves(board->bitboards[black_queen], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
    }
    return move_list;
}
