// Header file for board

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

#define CASTLE_WK 0b0001  // White king-side  (K)
#define CASTLE_WQ 0b0010  // White queen-side (Q)
#define CASTLE_BK 0b0100  // Black king-side  (k)
#define CASTLE_BQ 0b1000  // Black queen-side (q)
#define EP_NONE   0xFF  // No en passant available


// 104 Bytes total
struct Board {
  // Bitboards for each piece type
  uint64_t bitboards[12]; // 8 Bytes * 12 piece types = 96 Bytes 
  uint64_t white_occupied; // 1 Byte
  uint64_t black_occupied; // 1 Byte
  uint64_t all_occupied; //  1 Byte

  // ToDo! unite the castling rights with the en passant into a single byte
  unsigned char castling_rights; // 1 Byte
  unsigned char en_passant_square; // 1 Byte

  // Turn info
  bool player_turn; // 1 Byte
  unsigned char half_turn; // 1 Byte
  unsigned char counter_turn; // 1 Byte
};

// Player colors
enum {
  white_player = 0,
  black_player = 1,
};

// Piece indices
// White pieces: pair
// Black pieces: odd
enum {
  white_king = 0,
  black_king = 1,
  white_pawn = 2,
  black_pawn = 3,
  white_knight = 4,
  black_knight = 5,
  white_bishop = 6,
  black_bishop = 7,
  white_rook = 8,
  black_rook = 9,
  white_queen = 10,
  black_queen = 11,
};


void update_occupancy(struct Board* board);
struct Board fen_to_bitboards(char fen_string[]);

#endif


