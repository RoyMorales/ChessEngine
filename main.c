// Main Chess

#include <stdio.h>
#include "logic_board.c"

int main() { 
  char fen_setup[] = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr w KQkq - 0 1";
  //char fen_setup[] = "kkKkk w KQ - 20 30";
  
  struct Board initial_board = fen_to_bitboards(fen_setup); 
  print_bitboard(initial_board);

  return 0;
}
