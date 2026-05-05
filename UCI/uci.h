#ifndef UCI_H
#define UCI_H

#include "../core/board.h"
#include "../core/move_gen.h"

// Parse a UCI move string (e.g. "e2e4", "e7e8q") into a uint32_t move.
// Returns 0 if move not found in the legal move list.
uint32_t uci_parse_move(struct Board* board, const char* move_str);

// Convert a uint32_t move to UCI move string.
void uci_move_to_str(uint32_t move, char* out);

// Main UCI loop — reads from stdin, writes to stdout.
void uci_loop(void);

#endif