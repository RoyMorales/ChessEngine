// Logic for Chess Board

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

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


struct Board {
  uint64_t bitboards[12];
  
  char player_turn;

  bool white_rights[2];
  bool black_rights[2];

  char en_passant[2];

  unsigned char half_turn;
  unsigned char counter_turn;
};


void set_bit(uint64_t* number, int bit) {
  *number |= (1ULL << bit);
}

struct Board fen_to_bitboards(char fen_string[]) {
  struct Board board;
  memset(&board, 0, sizeof(board));

  uint64_t white_king_bitboard = 0, black_king_bitboard = 0;
  uint64_t white_pwan_bitboard = 0, black_pwan_bitboard = 0;
  uint64_t white_knight_bitboard = 0, black_knight_bitboard = 0;
  uint64_t white_bishop_bitboard = 0, black_bishop_bitboard = 0;
  uint64_t white_rook_bitboard = 0, black_rook_bitboard = 0;
  uint64_t white_queen_bitboard = 0, black_queen_bitboard = 0;

  char* token = strtok(fen_string, " ");
  int part = 0;
  while(token != NULL) {
    printf("Token: %s\n ", token);
    if (part == 0) {
      int square = 63;
      for(int i = 0; i < strlen(token); i++){
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
          case 'P': piece_index = white_pwan; break;
          case 'p': piece_index = black_pwan; break;
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
      for(int i = 0; i < strlen(token); i++) {
        char symbol = token[i];
        switch (symbol) {
          case 'K': board.white_rights[0] = true;
          case 'k': board.black_rights[0] = true;
          case 'Q': board.white_rights[1] = true;
          case 'q': board.black_rights[1] = true;
          default: continue;
        }
      }
      part++;
    }
     
    else if(part == 3) {
      if(token[0] == '-') {
        board.en_passant[0] = '0';
        board.en_passant[1] = '0';
        }
      else{
        board.en_passant[0] = token[0];
        board.en_passant[1] = token[1];
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

void print_bits(long long num) {
  for(int i = 1; i <= sizeof(num) * 8; i++) {
    printf("%d", (num >> ((sizeof(num) * 8) - i)) & 1);

    if (i % 8 == 0) {
      printf("\n");
    }
  }
}

void print_bitboard(struct Board board) {
  for(int i = 0; i < sizeof(board.bitboards) / sizeof(board.bitboards[0]); i++) {
    long long bitboard = board.bitboards[i];
      printf("\nBitboard: %d:\n", i);
      print_bits(bitboard);
  }
}

char* join_board_string(struct Board board) {
  static char board_string[65];
  for (int i = 0; i < 64; i++){
    board_string[i] = '.';
  } 
  board_string[64] = '\0'; /* ensure NUL terminator */

  size_t num_boards = sizeof(board.bitboards) / sizeof(board.bitboards[0]);
  const int BITS = sizeof(uint64_t) * 8;

  for(int i = 0; i < num_boards; i++) {
    uint64_t bitboard = (uint64_t)board.bitboards[i];

    for(int bit = 0; bit < BITS; bit++) {
      if(((bitboard >> bit) & 1ULL) == 0) continue;
      int idx = (BITS - 1 - bit);

        switch (i) {
          case white_king:   board_string[idx] = 'K'; break; 
          case black_king:   board_string[idx] = 'k'; break; 
          case white_pwan:   board_string[idx] = 'P'; break; 
          case black_pwan:   board_string[idx] = 'p'; break; 
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


