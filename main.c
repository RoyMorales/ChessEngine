// Main Chess

#include <stdio.h>
#include "logic_board.c"

int main() { 
  //char fen_setup[] = "RNBKQBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbkqbnr w KQkq - 0 1";
  //char fen_setup[] = "RNBKQBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbkqbnr w KQ - 20 30";
  char fen_setup[] = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"; 
  struct Board initial_board = fen_to_bitboards(fen_setup); 
  //print_bitboard(initial_board);

  char* board_string = join_board_string(initial_board);
  print_board_string(board_string);

  return 0;
}
