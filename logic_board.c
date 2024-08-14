// Logic for Chess Board

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

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
  long long bitboards[12];
  
  char player_turn;
  bool white_rights[2];
  bool black_rights[2];
  unsigned char half_turn;
  short int counter_turn;
};


void set_bit(long long* number, int bit) {
  *number |= (1LL << bit);
}

struct Board fen_to_bitboards(char fen_string[]) {
  struct Board board;

  long long white_king_bitboard = 0, black_king_bitboard = 0;
  long long white_pwan_bitboard = 0, black_pwan_bitboard = 0;
  long long white_knight_bitboard = 0, black_knight_bitboard = 0;
  long long white_bishop_bitboard = 0, black_bishop_bitboard = 0;
  long long white_rook_bitboard = 0, black_rook_bitboard = 0;
  long long white_queen_bitboard = 0, black_queen_bitboard = 0;

  board.bitboards[black_king] = black_king_bitboard;
  board.bitboards[white_pwan] = white_pwan_bitboard;
  board.bitboards[black_pwan] = black_pwan_bitboard;
  board.bitboards[white_knight] = white_knight_bitboard;
  board.bitboards[black_knight] = black_knight_bitboard;
  board.bitboards[white_bishop] = white_bishop_bitboard;
  board.bitboards[black_bishop] = black_bishop_bitboard;
  board.bitboards[white_rook] = white_rook_bitboard;
  board.bitboards[black_rook] = black_rook_bitboard;
  board.bitboards[white_queen] = white_queen_bitboard;
  board.bitboards[black_queen] = black_queen_bitboard;

  char player_turn;
  bool white_rights[2] = {false, false};
  bool black_rights[2] = {false, false};
  unsigned char half_turn;;
  short int counter_turn;

  char symbol;
  char* token = strtok(fen_string, " ");

  int part = 0;
  while(token != NULL) {
    printf("Token: %s\n ", token);
    if (part == 0) {
      int offset = 0;
      for(int i = 0; i < strlen(token); i++){
        symbol = token[i];

        switch(symbol) {
          case 'K': set_bit(&white_king_bitboard, i - offset); 
            board.bitboards[white_king] = white_king_bitboard;
            continue;
          case 'k': set_bit(&black_king_bitboard, i - offset); 
            board.bitboards[black_king] = black_king_bitboard;
            continue;
          case 'P': set_bit(&white_pwan_bitboard, i - offset); 
            board.bitboards[white_pwan] = white_pwan_bitboard;
            continue;
          case 'p': set_bit(&black_pwan_bitboard, i - offset); 
            board.bitboards[black_pwan] = black_pwan_bitboard;
            continue;
          case 'N': set_bit(&white_knight_bitboard, i - offset); 
            board.bitboards[white_knight] = white_knight_bitboard;
            continue;
          case 'n': set_bit(&black_knight_bitboard, i - offset); 
            board.bitboards[black_knight] = black_knight_bitboard;
            continue;
          case 'B': set_bit(&white_bishop_bitboard, i - offset); 
            board.bitboards[white_bishop] = white_bishop_bitboard;
            continue;
          case 'b': set_bit(&black_bishop_bitboard, i - offset); 
            board.bitboards[black_bishop] = black_bishop_bitboard;
            continue;
          case 'R': set_bit(&white_rook_bitboard, i - offset); 
            board.bitboards[white_rook] = white_rook_bitboard;
            continue;
          case 'r': set_bit(&black_rook_bitboard, i - offset); 
            board.bitboards[black_rook] = black_rook_bitboard;
            continue;
          case 'Q': set_bit(&white_queen_bitboard, i - offset); 
            board.bitboards[white_queen] = white_queen_bitboard;
            continue;
          case 'q': set_bit(&black_queen_bitboard, i - offset); 
            board.bitboards[black_queen] = black_queen_bitboard;
            continue;
          default: 
            if (isdigit(symbol)) {
              offset += symbol + 1;
              continue;
            } else {
            offset++; continue;
            }
        }
      }
      part++;
    }

    else if(part == 1) {
      symbol = token[0];
      switch (symbol) {
        case 'w': player_turn = 0;
        case 'b': player_turn = 1;
      }
      board.player_turn = player_turn;
      part++;
    }

    else if(part == 2) {
      for(int i = 0; i < strlen(token); i++) {
        symbol = token[i];

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
      //ToDo!
      part++;
    }

    else if(part == 4) {
      board.half_turn = symbol;
      part++;
    }

    else if(part == 5) {
      board.counter_turn = symbol;   
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
      printf("\nBitboard: %d: \n", i);
      print_bits(bitboard);
  }
}





