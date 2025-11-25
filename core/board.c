// Logic for Chess Board

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#include "logic.h"

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
  while(token != NULL) {
    printf("Token: %s\n ", token);
    if (part == 0) {
      int square = 63;
      for(size_t i = 0; i < strlen(token); i++){
        char symbol = token[i];

        if(symbol == '/') {
          continue;
        }
        if(isdigit(symbol)) {
          square -= symbol - '0';
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
          set_bit(&board.bitboards[piece_index], square);
          square--;
      }
      part++;
      }
    }

    else if(part == 1) {
      board.player_turn = token[0];
      part++;
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
      part++;
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
      part++;
    }

    else if(part == 4) {
      board.half_turn = token[0] - '0';
      part++;
    }

    else if(part == 5) {
      board.counter_turn = token[0] - '0';   
      part++;
    }
    token = strtok(NULL, " ");
  } 
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


