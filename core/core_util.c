// Utility functions for the Chess Engine

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "core_util.h"

// Function to clear the console screen
void clear_screen() {
  system("clear");
}

// Helper function to print bits of a number
void print_bits(uint64_t num) {
  for (int rank = 7; rank >= 0; rank--) { // rank 8 → rank 1
    for (int file = 0; file < 8; file++) { // file a → h
      int square = rank * 8 + file;
      printf("%zu", (num >> square) & 1);
    }
  printf("\n");
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

void print_move_list(struct MoveList* move_list) {
  printf("-----------------------------\n");
  printf("Move List (Total Moves: %d):\n", move_list->count);
  for (int i = 0; i < move_list->count; i++) {
    uint32_t move = move_list->moves[i];
    int from_square = move & 0x3F;               // Bits 0-5
    int to_square = (move >> 6) & 0x3F;          // Bits 6-11
    int promotion_piece = (move >> 12) & 0x0F;   // Bits 12-15
    int move_flags = (move >> 16) & 0x0F;        // Bits 16-19

    printf("Move %d: From %d To %d", i + 1, from_square, to_square);

    if (promotion_piece) {
      printf(" Promotion Piece: %d", promotion_piece);
    }
    if (move_flags) {
      printf(" Move Flags: %d", move_flags);
    }  
    printf("\n");
  }
  printf("-----------------------------\n");
}