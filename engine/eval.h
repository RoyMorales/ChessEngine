// Header File for evaluation module
 
#ifndef EVALUATION_H
#define EVALUATION_H
 
#include "../core/board.h"
#include "../core/move_gen.h"
#include "../core/move_filter.h"
#include "../core/move_apply.h"
 
int evaluation_material(struct Board* board);
int negamax(struct Board* board, int depth);
uint32_t find_best_move(struct Board* board, int depth);
 
#endif // EVALUATION_H