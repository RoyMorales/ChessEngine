// Header File for logic board

#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>

struct Board {
  uint64_t bitboards[12]; 
  char player_turn;
  bool white_rights[2];
  bool black_rights[2];
  char en_passant[2];
  unsigned char half_turn;
  unsigned char counter_turn;
};

enum {
  white_king = 0,
  black_king = 1,
  white_pwan = 2,
  black_pwan = 3,
  white_knight = 4,
  black_knight = 5,
  white_bishop = 6,
  black_bishop = 7,
  white_rook = 8,
  black_rook = 9,
  white_queen = 10,
  black_queen = 11,
};

struct Board fen_to_bitboards(char fen_string[]);
char* join_board_string(struct Board board);
void print_board_string(char* string_board);

#endif


