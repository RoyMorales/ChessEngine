// Header File for logic board

#ifdef LOGIC_H
#define LOGIC_H

void fen_to_bitboards();

typedef struct {
  long long bitboards[12];
  
  char player_turn;
  bool white_rights[2];
  bool black_rights[2];
  unsigned char half_turn;
  short int counter_turn;
} Board;

#endif


