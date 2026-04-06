// Board history implementation

#include <string.h>
#include <stdio.h>
#include "board_history.h"

void history_init(struct BoardHistory* history) {
    history->size = 0;
}

void history_push(struct BoardHistory* history, const struct Board* board, uint32_t move) {
    if (history->size >= MAX_HISTORY) {
        fprintf(stderr, "BoardHistory overflow — MAX_HISTORY (%d) exceeded\n", MAX_HISTORY);
        return;
    }
    history->boards[history->size] = *board;  // Full struct copy
    history->moves[history->size]  = move;
    history->size++;
}

bool history_pop(struct BoardHistory* history, struct Board* board) {
    if (history->size == 0) {
        return false;
    }
    history->size--;
    *board = history->boards[history->size];  // Restore full snapshot
    return true;
}

bool history_can_undo(const struct BoardHistory* history) {
    return history->size > 0;
}

// Two positions are considered equal if their bitboards, castling rights,
// en passant square, and player turn all match.
// (half_turn and counter_turn are ignored — they don't affect legality.)
static bool boards_equal(const struct Board* a, const struct Board* b) {
    if (a->player_turn      != b->player_turn)      return false;
    if (a->castling_rights  != b->castling_rights)  return false;
    if (a->en_passant_square != b->en_passant_square) return false;
    for (int i = 0; i < 12; i++) {
        if (a->bitboards[i] != b->bitboards[i]) return false;
    }
    return true;
}

int history_count_repetitions(const struct BoardHistory* history, const struct Board* board) {
    int count = 0;
    for (int i = 0; i < history->size; i++) {
        if (boards_equal(&history->boards[i], board)) {
            count++;
        }
    }
    return count;
}