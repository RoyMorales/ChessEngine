// Header File for filter moves

#ifndef MOVE_FILTER_H
#define MOVE_FILTER_H

#include "move_gen.h"
#include "board.h"
#include "move_apply.h"

struct MoveList generate_legal_moves(struct Board* board);
struct MoveList engine_search_moves(struct Board* board);

#endif

