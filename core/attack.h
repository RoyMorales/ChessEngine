// Header file for attack patterns

#ifndef ATTACK_H
#define ATTACK_H

#include <stdint.h>

// Precompute attack tables, only possible for king, pawn, and knight
extern uint64_t king_attacks[64];
extern uint64_t white_pawn_attacks[64];
extern uint64_t black_pawn_attacks[64];
extern uint64_t knight_attacks[64];

void init_attack_tables();
uint64_t orthogonal_attacks(int square, uint64_t occupancy);
uint64_t diagonal_attacks(int square, uint64_t occupancy);

#endif