// Header File for logic board

#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <stdbool.h>

#define CASTLE_WK 0b0001  // White king-side  (K)
#define CASTLE_WQ 0b0010  // White queen-side (Q)
#define CASTLE_BK 0b0100  // Black king-side  (k)
#define CASTLE_BQ 0b1000  // Black queen-side (q)
#define EP_NONE   0xFF  // No en passant available


struct Board {
  // Bitboards for each piece type
  uint64_t bitboards[12]; 
  uint64_t white_occupied;
  uint64_t black_occupied;
  uint64_t all_occupied;

  // ToDo! unity the castling rights with the en passant into a single byte
  unsigned char castling_rights;
  unsigned char en_passant_square;

  // Turn info
  char player_turn;
  unsigned char half_turn;
  unsigned char counter_turn;
};

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
char* join_board_string(struct Board board);
void print_board_string(char* string_board);

#endif


