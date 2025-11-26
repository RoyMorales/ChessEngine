// Precompute attack patterns for king, pawn, and knight 
// to improve efficiency of move generation
// Compute sliding attacks for rooks and bishops dynamically

#include <stdint.h>

#include "attack.h"

uint64_t king_attacks[64];
uint64_t white_pawn_attacks[64];
uint64_t black_pawn_attacks[64];
uint64_t knight_attacks[64];

// Precompute every possible king attack from each square
void precompute_king_attacks(uint64_t* table) {
  const int king_moves[8][2] = {
    {1, 0}, {1, 1}, {0, 1}, {-1, 1},
    {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
  };

  for (int square = 0; square < 64; square++) {
    uint64_t attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    for (int i = 0; i < 8; i++) {
      int new_rank = rank + king_moves[i][0];
      int new_file = file + king_moves[i][1];

      if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
        int attack_square = new_rank * 8 + new_file;
        attacks |= (1ULL << attack_square);
      }
    }
    table[square] = attacks;
  }
}

//Precompute every possible white pawn attack from each square
void precompute_white_pawn_attacks(uint64_t* table) {
  for (int square = 0; square < 64; square++) {
    uint64_t attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    if (rank < 7) { // Not on the last rank
      if (file > 0) { // Can attack to the left
        attacks |= (1ULL << ((rank + 1) * 8 + (file - 1)));
      }
      if (file < 7) { // Can attack to the right
        attacks |= (1ULL << ((rank + 1) * 8 + (file + 1)));
      }
    }
    table[square] = attacks;
  }
}

void precompute_black_pawn_attacks(uint64_t* table) {
  for (int square = 0; square < 64; square++) {
    uint64_t attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    if (rank > 0) { // Not on the first rank
      if (file > 0) { // Can attack to the left
        attacks |= (1ULL << ((rank - 1) * 8 + (file - 1)));
      }
      if (file < 7) { // Can attack to the right
        attacks |= (1ULL << ((rank - 1) * 8 + (file + 1)));
      }
    }
    table[square] = attacks;
  }
}


// Precompute every possible knight attack from each square
void precompute_knight_attacks(uint64_t* table) {
  const int knight_moves[8][2] = {
    {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
    {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
  };

  for (int square = 0; square < 64; square++) {
    uint64_t attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    for (int i = 0; i < 8; i++) {
      int new_rank = rank + knight_moves[i][0];
      int new_file = file + knight_moves[i][1];

      if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
        int attack_square = new_rank * 8 + new_file;
        attacks |= (1ULL << attack_square);
      }
    }
    table[square] = attacks;
  }
}

// Sliding orthogonal attacks (rook-like)
uint64_t orthogonal_attacks(int square, uint64_t occupancy) {
    uint64_t attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    int orthogonal_directions[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    for (int i = 0; i < 4; i++) {
        int direction_rank = orthogonal_directions[i][0];
        int direction_file = orthogonal_directions[i][1];
        int new_rank = rank + direction_rank;
        int new_file = file + direction_file;

        while (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
            int attack_square = new_rank * 8 + new_file;
            attacks |= (1ULL << attack_square);

            if (occupancy & (1ULL << attack_square)) {
                break; // Stop if there's a blocking piece
            }

            new_rank += direction_rank;
            new_file += direction_file;
        }
    } 
    return attacks;
}

// Sliding diagonal attacks (bishop-like)
uint64_t diagonal_attacks(int square, uint64_t occupancy) {
    uint64_t attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    int diagonal_directions[4][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (int i = 0; i < 4; i++) {
        int direction_rank = diagonal_directions[i][0];
        int direction_file = diagonal_directions[i][1];
        int new_rank = rank + direction_rank;
        int new_file = file + direction_file;

        while (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
            int attack_square = new_rank * 8 + new_file;
            attacks |= (1ULL << attack_square);

            if (occupancy & (1ULL << attack_square)) {
                break; // Stop if there's a blocking piece
            }

            new_rank += direction_rank;
            new_file += direction_file;
        }
    } 
    return attacks;
}

// Initialize all attack tables
void init_attack_tables() {
  precompute_king_attacks(king_attacks);
  precompute_white_pawn_attacks(white_pawn_attacks);
  precompute_black_pawn_attacks(black_pawn_attacks);
  precompute_knight_attacks(knight_attacks);
}


