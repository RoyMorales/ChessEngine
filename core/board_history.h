// Board history: stack of full Board snapshots for undo and repetition detection

#ifndef BOARD_HISTORY_H
#define BOARD_HISTORY_H

#include <stdbool.h>
#include <stdint.h>
#include "board.h"

#define MAX_HISTORY 1024  // Enough for any real game + engine search depth

struct BoardHistory {
    struct Board boards[MAX_HISTORY];
    uint32_t     moves[MAX_HISTORY];   // The move that led to boards[i+1]
    int          size;                 // Number of entries currently in the stack
};

// Initialise an empty history
void history_init(struct BoardHistory* history);

// Push a snapshot of the current board + the move about to be made
// Call this BEFORE apply_move()
void history_push(struct BoardHistory* history, const struct Board* board, uint32_t move);

// Pop the last snapshot and restore it into board.
// Returns true on success, false if the history is empty (already at start).
bool history_pop(struct BoardHistory* history, struct Board* board);

// Returns true if there is at least one move to undo
bool history_can_undo(const struct BoardHistory* history);

// Returns the number of times the given board position has appeared in history
// (used for threefold-repetition detection later)
int history_count_repetitions(const struct BoardHistory* history, const struct Board* board);

#endif