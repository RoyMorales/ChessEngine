// Header File for filter moves

#ifndef MOVE_FILTER_H
#define MOVE_FILTER_H

#include "move_gen.h"
#include "board.h"
#include "move_apply.h"

bool is_square_attacked(struct Board* board, int square, bool by_white);
int  get_king_square(struct Board* board, bool player_colour);
bool is_legal_move(struct Board* board, uint32_t move);

struct MoveList generate_legal_moves(struct Board* board);
struct MoveList engine_search_moves(struct Board* board);

#endif
