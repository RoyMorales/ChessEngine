// Make and Unmake moves for search tree

#include "move_stack.h"
#include "../core/board.h"
#include "../core/move_gen.h"

void push_move(struct Board* board, struct MoveInfo* move_info, uint32_t move) {
    int from = move & 0x3F;
    int to   = (move >> 6) & 0x3F;

    bool is_capture= (move >> CAPTURE) & 1;
    bool is_castle = (move >> MOVE_CASTLING) & 1;
    bool is_ep     = (move >> EN_PASSANT) & 1;

    int side = board->player_turn;

    // Save move info
    move_info->capture_piece_bitboard = 0;
    move_info->capture_piece_type = -1;

    move_info->castling_rights = board->castling_rights;
    move_info->en_passant_square = board->en_passant_square;
    move_info->half_turn = board->half_turn;

    uint64_t from_bitboard = (1ULL << from);
    uint64_t to_bitboard   = (1ULL << to);

    // Identify captured piece
    // Save the piece type into the remaining bits in move
    // Future work
    int piece_type = -1;
    for (int piece = side; piece < 12; piece += 2) {
        if (board->bitboards[piece] & (1ULL << from)) {
            piece_type = piece;
            break;
        }
    }

    int capture_square = 0;
    if (is_capture) {
        
        if(is_ep) {
           capture_square = (side == white_player) ? to - 8 : to + 8;
        } else {
           capture_square = to;
        }

        uint64_t capture_bitboard = (1ULL << capture_square);

        for (int piece = side; piece < 12; piece += 2) {
            if (board->bitboards[piece] & capture_bitboard) {
                board->bitboards[piece] &~ capture_bitboard;
                move_info->capture_piece_bitboard = capture_bitboard;
                move_info->capture_piece_type = piece;
                break;
            }
        }
        board->half_turn = 0; // Reset half turn on capture
    }

    if (is_castle) {
        if (to == 62)      board->bitboards[white_rook] ^= (1ULL<<63)|(1ULL<<61);
        else if (to == 58) board->bitboards[white_rook] ^= (1ULL<<56)|(1ULL<<59);
        else if (to == 6)  board->bitboards[black_rook] ^= (1ULL<<7)|(1ULL<<5);
        else if (to == 2)  board->bitboards[black_rook] ^= (1ULL<<0)|(1ULL<<3);
    }

    board->bitboards[piece_type] &= ~from_bitboard;
    board->bitboards[piece_type] |= to_bitboard;
    
    // Update Board
    board->en_passant_square = EP_NONE;
    if ((move >> DOUBLE_PUSH) & 1) board->en_passant_square = side == white_player ? to -8 : to + 8;

        // Update castling rights
            // King moves
    if (piece_type == white_king) {
        board->castling_rights &= ~(CASTLE_WK | CASTLE_WQ);
    } else if (piece_type == black_king) {
        board->castling_rights &= ~(CASTLE_BK | CASTLE_BQ);
    }
            // Rook moves
    if (piece_type == white_rook) {
        if (from == 63) board->castling_rights &= ~CASTLE_WK;
        else if (from == 56) board->castling_rights &= ~CASTLE_WQ;
    } else if (piece_type == black_rook) {
        if (from == 7) board->castling_rights &= ~CASTLE_BK;
        else if (from == 0) board->castling_rights &= ~CASTLE_BQ;
    }
            // Rook captures
    if (is_capture) {
        if (capture_square == 63) board->castling_rights &= ~CASTLE_WK;
        else if (capture_square == 56) board->castling_rights &= ~CASTLE_WQ;
        else if (capture_square == 7) board->castling_rights &= ~CASTLE_BK;
        else if (capture_square == 0) board->castling_rights &= ~CASTLE_BQ;
    } 

    update_occupancy(board);
    board->player_turn = !side;
}
