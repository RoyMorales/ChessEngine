// Utility functions for the Chess Engine

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "board.h"

// Helper function to print bits of a number
void print_bits(long long num) {
  for(size_t i = 1; i <= sizeof(num) * 8; i++) {
    printf("%lld", (num >> ((sizeof(num) * 8) - i)) & 1);

    if (i % 8 == 0) {
      printf("\n");
    }
  }
}

// Function to print all bitboards in the Board struct
void print_bitboard(struct Board board) {
  for(size_t i = 0; i < sizeof(board.bitboards) / sizeof(board.bitboards[0]); i++) {
    long long bitboard = board.bitboards[i];
      printf("\nBitboard: %zu:\n", i);
      print_bits(bitboard);
  }
}

// Function to convert Board struct to a string representation
char* join_board_string(struct Board board) {
  static char board_string[65];
  for (int i = 0; i < 64; i++){
    board_string[i] = '.';
  } 
  board_string[64] = '\0'; /* ensure NUL terminator */

  size_t num_boards = sizeof(board.bitboards) / sizeof(board.bitboards[0]);
  const int BITS = sizeof(uint64_t) * 8;

  for(size_t i = 0; i < num_boards; i++) {
    uint64_t bitboard = (uint64_t)board.bitboards[i];

    for(int bit = 0; bit < BITS; bit++) {
      if(((bitboard >> bit) & 1ULL) == 0) continue;
      int idx = (BITS - 1 - bit);

        switch (i) {
          case white_king:   board_string[idx] = 'K'; break; 
          case black_king:   board_string[idx] = 'k'; break; 
          case white_pawn:   board_string[idx] = 'P'; break; 
          case black_pawn:   board_string[idx] = 'p'; break; 
          case white_knight: board_string[idx] = 'N'; break; 
          case black_knight: board_string[idx] = 'n'; break; 
          case white_bishop: board_string[idx] = 'B'; break; 
          case black_bishop: board_string[idx] = 'b'; break; 
          case white_rook:   board_string[idx] = 'R'; break; 
          case black_rook:   board_string[idx] = 'r'; break; 
          case white_queen:  board_string[idx] = 'Q'; break; 
          case black_queen:  board_string[idx] = 'q'; break; 
          default: board_string[idx] = '.'; break;
        }
      }
    }
  return board_string;
}

// Function to print the board string in a readable format
void print_board_string(char* string_board) {
  printf("\n");
  for(int i = 0; i < (int)strlen(string_board); i++) {
    char symbol = string_board[i];
    printf("|%c", symbol);
    if ((i + 1) % 8 == 0) {
      printf("|\n");
    }
  } 
}