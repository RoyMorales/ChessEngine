// Generate move attack for pieces

#include "board.h"
#include "attack.h"
#include "movegen.h"
#include "core_util.h"

void generate_white_pawn_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from_square = __builtin_ctzll(pawns);
        int to_square = from_square + 8;

        if (to_square < 64 && !(occupancy & (1ULL << to_square))) {
            uint32_t move = 0;
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            move_list->moves[move_list->count++] = move;
        }
        pawns &= pawns - 1; // Clear the least significant bit
    }
}

void generate_black_pawn_push_moves(uint64_t pawns, struct MoveList* move_list, uint64_t occupancy) {
    while (pawns) {
        int from_square = __builtin_ctzll(pawns);
        int to_square = from_square - 8;

        if (to_square >= 0 && !(occupancy & (1ULL << to_square))) {
            uint32_t move = 0;
            move |= from_square;               // From square
            move |= (to_square << 6);         // To square

            move_list->moves[move_list->count++] = move;
        }
        pawns &= pawns - 1; // Clear the least significant bit
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

void generate_white_pawn_capture_moves(uint64_t pawns, struct MoveList* move_list, uint64_t opponent_pieces) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1; // Clear the least significant bit

        uint64_t attacks = white_pawn_attacks[from];
        uint64_t captures = attacks & opponent_pieces;

        while (captures) {
            int to = __builtin_ctzll(captures);
            captures &= captures - 1;

            uint32_t move = 0;
            move |= from;           // from square
            move |= (to << 6);      // to square
            move |= (1 << 17);      // Capture flag

            move_list->moves[move_list->count++] = move;
        }
    }
}

void generate_black_pawn_capture_moves(uint64_t pawns, struct MoveList* move_list, uint64_t opponent_pieces) {
    while (pawns) {
        int from = __builtin_ctzll(pawns);
        pawns &= pawns - 1; // Clear the least significant bit

        uint64_t attacks = black_pawn_attacks[from];
        uint64_t captures = attacks & opponent_pieces;

        while (captures) {
            int to = __builtin_ctzll(captures);
            captures &= captures - 1;

            uint32_t move = 0;
            move |= from;           // from square
            move |= (to << 6);      // to square
            move |= (1 << 17);      // Capture flag

            move_list->moves[move_list->count++] = move;
        }
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




struct MoveList genrate_board_moves(struct Board* board) {
    struct MoveList move_list;
    move_list.count = 0;

    uint64_t own_pieces = (board->player_turn == white_player) ? board->white_occupied : board->black_occupied;
    uint64_t opponent_pieces = (board->player_turn == white_player) ? board->black_occupied : board->white_occupied;

    if (board->player_turn == white_player) {
        generate_white_pawn_moves(board->bitboards[white_pawn], &move_list, board->all_occupied, opponent_pieces);
        generate_knight_moves(board->bitboards[white_knight], &move_list, own_pieces, opponent_pieces);
        
    } else {
        generate_black_pawn_moves(board->bitboards[black_pawn], &move_list, board->all_occupied, opponent_pieces);
        generate_knight_moves(board->bitboards[black_knight], &move_list, own_pieces, opponent_pieces);
    }
    return move_list;
}

