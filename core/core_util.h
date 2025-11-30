// Header File for utility functions

#ifndef CORE_UTIL_H
#define CORE_UTIL_H

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "movegen.h"


void clear_screen();
void print_bits(uint64_t bitboard);
void print_bitboard(struct Board board);
char* join_board_string(struct Board board);
void print_board_string(char* string_board);
void print_move_list(struct MoveList* move_list);

#endif

