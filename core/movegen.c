// Generate move attack for pieces

#include "board.h"
#include "attack.h"
#include "movegen.h"
#include "core_util.h"


void generate_king_moves(uint64_t kings, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces) {
    while (kings) {
        int from_square = __builtin_ctzll(kings);
        uint64_t attacks = king_attacks[from_square] & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << 17); // Capture flag
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
        pawns &= pawns - 1;

        int to = from + 8;
        if (to >= 64 || (occupancy & (1ULL << to))) continue;

        // Promotion
        if (to >= 56) {
            int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
            for (int i = 0; i < 4; i++) {
                uint32_t move = 0;
                move |= from;
                move |= (to << 6);
                move |= (promo_pieces[i] << 12);
                move_list->moves[move_list->count++] = move;
            }
        } else {
            uint32_t move = 0;
            move |= from;
            move |= (to << 6);
            move_list->moves[move_list->count++] = move;
        }
    }
}


void generate_black_pawn_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        int to = from - 8;
        if (to < 0 || (occupancy & (1ULL << to))) continue;

        // Promotion
        if (to <= 7) {
            int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
            for (int i = 0; i < 4; i++) {
                uint32_t move = 0;
                move |= from;
                move |= (to << 6);
                move |= (promo_pieces[i] << 12);
                move_list->moves[move_list->count++] = move;
            }
        } else {
            uint32_t move = 0;
            move |= from;
            move |= (to << 6);
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
            captures &= captures - 1;

            // PROMOTION CAPTURE
            if (to >= 56) {
                int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
                for (int i = 0; i < 4; i++) {
                    uint32_t move = 0;
                    move |= from;
                    move |= (to << 6);
                    move |= (1 << 17);  // capture
                    move |= (promo_pieces[i] << 12);
                    move_list->moves[move_list->count++] = move;
                }
            } else {
                uint32_t move = 0;
                move |= from;
                move |= (to << 6);
                move |= (1 << 17); // capture
                move_list->moves[move_list->count++] = move;
            }
        }
    }
}


void generate_black_pawn_capture_moves(uint64_t pawns, struct MoveList* move_list, uint64_t opponent_pieces) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        uint64_t captures = white_pawn_attacks[from] & opponent_pieces;

        while (captures) {
            int to = __builtin_ctzll(captures);
            captures &= captures - 1;

            // PROMOTION CAPTURE
            if (to >= 7) {
                int promo_pieces[4] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
                for (int i = 0; i < 4; i++) {
                    uint32_t move = 0;
                    move |= from;
                    move |= (to << 6);
                    move |= (1 << 17);  // capture
                    move |= (promo_pieces[i] << 12);
                    move_list->moves[move_list->count++] = move;
                }
            } else {
                uint32_t move = 0;
                move |= from;
                move |= (to << 6);
                move |= (1 << 17); // capture
                move_list->moves[move_list->count++] = move;
            }
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
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square
            move |= (1 << 16);                 // Double push flag

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
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square
            move |= (1 << 16);                 // Double push flag

            move_list->moves[move_list->count++] = move;
        }
        pawns &= pawns - 1; // Clear the least significant bit
    }
}


void generate_white_pawn_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy, uint64_t opponent_pieces) {
    generate_white_pawn_push_moves(pawns, move_list, occupancy);
    generate_white_pawn_double_push_moves(pawns, move_list, occupancy);
    generate_white_pawn_capture_moves(pawns, move_list, opponent_pieces);
}

void generate_black_pawn_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy, uint64_t opponent_pieces) {
    generate_black_pawn_push_moves(pawns, move_list, occupancy);
    generate_black_pawn_double_push_moves(pawns, move_list, occupancy);
    generate_black_pawn_capture_moves(pawns, move_list, opponent_pieces);
}

void generate_knight_moves(uint64_t knights, struct MoveList* move_list, uint64_t own_pieces, uint64_t opponent_pieces) {
    while (knights) {
        int from_square = __builtin_ctzll(knights);
        uint64_t attacks = knight_attacks[from_square] & ~own_pieces;

        while (attacks) {
            int to_square = __builtin_ctzll(attacks);

            uint32_t move = 0;
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << 17); // Capture flag
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
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << 17); // Capture flag
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
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << 17); // Capture flag
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
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            if (opponent_pieces & (1ULL << to_square)) {
                move |= (1 << 17); // Capture flag
            }

            move_list->moves[move_list->count++] = move;

            attacks &= attacks - 1; // Clear the least significant bit
        }

        queens &= queens - 1; // Clear the least significant bit
    }
}


struct MoveList genrate_board_moves(struct Board* board) {
    struct MoveList move_list;
    move_list.count = 0;

    uint64_t own_pieces = (board->player_turn == white_player) ? board->white_occupied : board->black_occupied;
    uint64_t opponent_pieces = (board->player_turn == white_player) ? board->black_occupied : board->white_occupied;

    if (board->player_turn == white_player) {
        generate_king_moves(board->bitboards[white_king], &move_list, own_pieces, opponent_pieces);
        generate_white_pawn_moves(board->bitboards[white_pawn], &move_list, board->all_occupied, opponent_pieces);
        generate_knight_moves(board->bitboards[white_knight], &move_list, own_pieces, opponent_pieces);
        generate_bishop_moves(board->bitboards[white_bishop], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
        generate_rook_moves(board->bitboards[white_rook], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
        generate_queen_moves(board->bitboards[white_queen], &move_list, board->white_occupied, board->black_occupied, board->all_occupied);
    } else {
        generate_king_moves(board->bitboards[black_king], &move_list, own_pieces, opponent_pieces);
        generate_black_pawn_moves(board->bitboards[black_pawn], &move_list, board->all_occupied, opponent_pieces);
        generate_knight_moves(board->bitboards[black_knight], &move_list, own_pieces, opponent_pieces);
        generate_bishop_moves(board->bitboards[black_bishop], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
        generate_rook_moves(board->bitboards[black_rook], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
        generate_queen_moves(board->bitboards[black_queen], &move_list, board->black_occupied, board->white_occupied, board->all_occupied);
    }
    return move_list;
}

