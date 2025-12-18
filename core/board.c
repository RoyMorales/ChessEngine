// Logic for Chess Board

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#include "board.h"

// Helper function to set a specific bit in a number
void set_bit(uint64_t* number, int bit) {
  *number |= (1ULL << bit);
}

// Helper function to convert algebraic notation to square index
static unsigned char square_from_alg(const char *s) {
  if (!s || s[0] == '\0') return EP_NONE;
    char file = s[0];
    char rank = s[1];
  if (file >= 'A' && file <= 'H')
    file = file - 'A' + 'a'; // allow uppercase
  if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
    printf("Invalid en passant square: %s\n", s);
    return EP_NONE;
  }
  unsigned int f = (unsigned int)(file - 'a');
  unsigned int r = (unsigned int)(rank - '1');
  return (unsigned char)(r * 8 + f);
}

// Function to convert FEN string to Board struct
struct Board fen_to_bitboards(char fen_string[]) {
  struct Board board;
  memset(&board, 0, sizeof(board));

  char* token = strtok(fen_string, " ");
  int part = 0;

  printf("\n----------------------------\n");
  while(token != NULL) {

    if (part == 0) {
      int rank = 7;
      int file = 0;
      for(size_t i = 0; i < strlen(token); i++){
        char symbol = token[i];

        if(symbol == '/') {
          rank--;
          file = 0;
          continue;
        }
        if(isdigit(symbol)) {
          file += symbol - '0';
          continue;
        }

        int piece_index = -1;
        switch (symbol) {
          case 'K': piece_index = white_king; break;
          case 'k': piece_index = black_king; break;
          case 'P': piece_index = white_pawn; break;
          case 'p': piece_index = black_pawn; break;
          case 'N': piece_index = white_knight; break;
          case 'n': piece_index = black_knight; break;
          case 'B': piece_index = white_bishop; break;
          case 'b': piece_index = black_bishop; break;
          case 'R': piece_index = white_rook; break;
          case 'r': piece_index = black_rook; break;
          case 'Q': piece_index = white_queen; break;
          case 'q': piece_index = black_queen; break;
          default: break;
        }
        if (piece_index >= 0) {
          int square = rank * 8 + file;
          set_bit(&board.bitboards[piece_index], square);
          file++;
        }
      }
      printf("Finished parsing piece placement.\n");
    }

    else if(part == 1) {
      board.player_turn = (token[0] == 'w') ? 0 : 1;
      printf("Player Turn: %s\n", board.player_turn == 0 ? "White" : "Black");
    }

    else if(part == 2) {
      board.castling_rights = 0;
      for (size_t i = 0; i < strlen(token); i++) {
        char symbol = token[i];
        switch (symbol) {
          case 'K': board.castling_rights |= CASTLE_WK; break;
          case 'Q': board.castling_rights |= CASTLE_WQ; break;
          case 'k': board.castling_rights |= CASTLE_BK; break;
          case 'q': board.castling_rights |= CASTLE_BQ; break;
          case '-': break;
          default: break;
        }
      }
      printf("Castling Rights: %u\n", board.castling_rights);
    }

    else if(part == 3) {
      if (token[0] == '-' && token[1] == '\0') {
        board.en_passant_square = EP_NONE;
      } else {
        if (strlen(token) >= 2) {
          unsigned char sq = square_from_alg(token);
          board.en_passant_square = (sq == EP_NONE) ? EP_NONE : sq;
        } else {
          board.en_passant_square = EP_NONE;
        }
      }
      printf("En Passant Square: %u\n", board.en_passant_square);
    }

    else if(part == 4) {
      board.half_turn = token[0] - '0';
      printf("Half Turn: %u\n", board.half_turn);
    }

    else if(part == 5) {
      board.counter_turn = token[0] - '0';
      printf("Counter Turn: %u\n", board.counter_turn);
    }

    part ++;
    token = strtok(NULL, " ");
  }
  printf("----------------------------\n\n");
  return board;
}

// Function to update occupancy bitboards
void update_occupancy(struct Board* board)
{
    board->white_occupied =
        board->bitboards[white_pawn]  |
        board->bitboards[white_knight]|
        board->bitboards[white_bishop]|
        board->bitboards[white_rook]  |
        board->bitboards[white_queen] |
        board->bitboards[white_king];

    board->black_occupied =
        board->bitboards[black_pawn]  |
        board->bitboards[black_knight]|
        board->bitboards[black_bishop]|
        board->bitboards[black_rook]  |
        board->bitboards[black_queen] |
        board->bitboards[black_king];

    board->all_occupied =
        board->white_occupied | board->black_occupied;
}
