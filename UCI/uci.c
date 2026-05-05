// UCI (Universal Chess Interface) protocol implementation
// Spec: https://www.shredderchess.com/chess-features/uci-universal-chess-interface.html
//
// Communication is via stdin/stdout, line by line.
// All output must be flushed immediately — GUIs are sensitive to buffering.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "uci.h"
#include "../core/core_util.h"
#include "../core/board.h"
#include "../core/attack.h"
#include "../core/move_gen.h"
#include "../core/move_filter.h"
#include "../core/move_apply.h"
#include "../core/zobrist.h"
#include "../engine/eval.h"

#define ENGINE_NAME    "CodFish"
#define ENGINE_AUTHOR  "Roy Morales"
#define START_FEN      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAX_DEPTH      64
#define DEFAULT_DEPTH  6
#define INPUT_BUF      4096

// ---------------------------------------------------------------------------
// Move string conversion
// ---------------------------------------------------------------------------

void uci_move_to_str(uint32_t move, char* out) {
    int from  = move & 0x3F;
    int to    = (move >> 6) & 0x3F;
    int promo = (move >> 12) & 0xF;

    out[0] = 'a' + from % 8;
    out[1] = '1' + from / 8;
    out[2] = 'a' + to % 8;
    out[3] = '1' + to / 8;

    if (promo) {
        const char pc[] = {0, 'q', 'r', 'b', 'n'};
        out[4] = (promo < 5) ? pc[promo] : 'q';
        out[5] = '\0';
    } else {
        out[4] = '\0';
    }
}

uint32_t uci_parse_move(struct Board* board, const char* move_str) {
    if (!move_str || strlen(move_str) < 4) return 0;

    int from_file = move_str[0] - 'a';
    int from_rank = move_str[1] - '1';
    int to_file   = move_str[2] - 'a';
    int to_rank   = move_str[3] - '1';

    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
        to_file   < 0 || to_file   > 7 || to_rank   < 0 || to_rank   > 7)
        return 0;

    int from = from_rank * 8 + from_file;
    int to   = to_rank   * 8 + to_file;

    // Optional promotion piece
    int promo = 0;
    if (move_str[4]) {
        switch (move_str[4]) {
            case 'q': promo = PROMOTION_QUEEN;  break;
            case 'r': promo = PROMOTION_ROOK;   break;
            case 'b': promo = PROMOTION_BISHOP; break;
            case 'n': promo = PROMOTION_KNIGHT; break;
        }
    }

    // Match against legal moves — find the one with matching from/to/promo
    struct MoveList moves = generate_legal_moves(board);
    for (int i = 0; i < moves.count; i++) {
        uint32_t m    = moves.moves[i];
        int      mf   = m & 0x3F;
        int      mt   = (m >> 6) & 0x3F;
        int      mp   = (m >> 12) & 0xF;

        if (mf == from && mt == to) {
            // For promotions, match the piece; for non-promotions accept any
            if (promo == 0 || mp == promo) return m;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// "position" command parser
// Syntax: position [startpos | fen <fenstring>] [moves <move1> ... <moveN>]
// ---------------------------------------------------------------------------
static struct Board parse_position(const char* line) {
    struct Board board;

    if (strncmp(line, "position startpos", 17) == 0) {
        char fen_buf[] = START_FEN;  // mutable copy — fen_to_bitboards modifies input
        board = fen_to_bitboards(fen_buf);
        update_occupancy(&board);
        line += 17;
    } else if (strncmp(line, "position fen", 12) == 0) {
        line += 13;  // skip "position fen "
        // Copy into mutable buffer — fen_to_bitboards modifies its input in-place
        char fen_buf[256];
        strncpy(fen_buf, line, 255);
        fen_buf[255] = '\0';
        board = fen_to_bitboards(fen_buf);
        update_occupancy(&board);
        // Advance past the FEN (6 space-separated fields)
        int spaces = 0;
        while (*line && spaces < 6) {
            if (*line == ' ') spaces++;
            line++;
        }
    } else {
        char fen_buf[] = START_FEN;
        board = fen_to_bitboards(fen_buf);
        update_occupancy(&board);
    }

    // Apply moves if present
    const char* moves_ptr = strstr(line, "moves");
    if (moves_ptr) {
        moves_ptr += 6;  // skip "moves "
        char move_str[8];
        while (*moves_ptr) {
            while (*moves_ptr == ' ') moves_ptr++;
            if (!*moves_ptr) break;

            int i = 0;
            while (*moves_ptr && *moves_ptr != ' ' && i < 7)
                move_str[i++] = *moves_ptr++;
            move_str[i] = '\0';
            if (i == 0) break;

            uint32_t move = uci_parse_move(&board, move_str);
            if (move) {
                apply_move(&board, move);
                update_occupancy(&board);
            }
        }
    }

    return board;
}

// ---------------------------------------------------------------------------
// "go" command parser
// Syntax: go [depth <d>] [movetime <ms>] [wtime <ms>] [btime <ms>]
//            [winc <ms>] [binc <ms>] [infinite]
// ---------------------------------------------------------------------------
typedef struct {
    int    depth;
    int    movetime_ms;   // fixed time per move (ms), -1 = not set
    int    wtime_ms;
    int    btime_ms;
    int    winc_ms;
    int    binc_ms;
    bool   infinite;
} GoParams;

static GoParams parse_go(const char* line) {
    GoParams p = {
        .depth       = DEFAULT_DEPTH,
        .movetime_ms = -1,
        .wtime_ms    = -1,
        .btime_ms    = -1,
        .winc_ms     = 0,
        .binc_ms     = 0,
        .infinite    = false,
    };

    const char* tok = line + 3;  // skip "go "
    while (*tok) {
        while (*tok == ' ') tok++;
        if (!*tok) break;

        if (strncmp(tok, "depth ", 6) == 0)       { p.depth       = atoi(tok + 6); }
        else if (strncmp(tok, "movetime ", 9) == 0){ p.movetime_ms = atoi(tok + 9); }
        else if (strncmp(tok, "wtime ", 6) == 0)   { p.wtime_ms    = atoi(tok + 6); }
        else if (strncmp(tok, "btime ", 6) == 0)   { p.btime_ms    = atoi(tok + 6); }
        else if (strncmp(tok, "winc ", 5) == 0)    { p.winc_ms     = atoi(tok + 5); }
        else if (strncmp(tok, "binc ", 5) == 0)    { p.binc_ms     = atoi(tok + 5); }
        else if (strncmp(tok, "infinite", 8) == 0) { p.infinite    = true; p.depth = MAX_DEPTH; }

        // Advance to next token
        while (*tok && *tok != ' ') tok++;
    }

    return p;
}

// Compute time to search in seconds from go parameters
static double compute_time_limit(GoParams* p, bool white_to_move) {
    if (p->movetime_ms > 0)
        return p->movetime_ms / 1000.0;

    if (p->infinite)
        return 1e9;  // effectively unlimited

    int clock_ms = white_to_move ? p->wtime_ms : p->btime_ms;
    int inc_ms   = white_to_move ? p->winc_ms  : p->binc_ms;

    if (clock_ms <= 0)
        return 5.0;  // fallback

    // Simple time management: use ~1/30 of remaining time + increment
    // This is conservative but safe for a first implementation
    double alloc = (clock_ms / 30.0 + inc_ms * 0.8) / 1000.0;

    // Never use more than 20% of remaining clock
    double max_alloc = clock_ms * 0.2 / 1000.0;

    return (alloc < max_alloc) ? alloc : max_alloc;
}

// ---------------------------------------------------------------------------
// UCI main loop
// ---------------------------------------------------------------------------
void uci_loop(void) {
    // Disable stdout buffering — GUIs require immediate output
    setbuf(stdout, NULL);

    // Suppress board debug output — UCI uses stdout for protocol messages only
    board_quiet_mode = true;

    init_attack_tables();
    zobrist_init();

    char start_fen[] = START_FEN;
    struct Board board = fen_to_bitboards(start_fen);
    update_occupancy(&board);

    char line[INPUT_BUF];

    while (fgets(line, INPUT_BUF, stdin)) {
        // Strip trailing newline/CR
        line[strcspn(line, "\r\n")] = '\0';

        // ---------------------------------------------------------------
        // uci — identify the engine
        // ---------------------------------------------------------------
        if (strcmp(line, "uci") == 0) {
            printf("id name %s\n", ENGINE_NAME);
            printf("id author %s\n", ENGINE_AUTHOR);
            printf("option name Hash type spin default 128 min 1 max 4096\n");
            printf("option name Threads type spin default 1 min 1 max 1\n");
            printf("uciok\n");
            fflush(stdout);
        }

        // ---------------------------------------------------------------
        // isready — engine must respond "readyok" when ready
        // ---------------------------------------------------------------
        else if (strcmp(line, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);
        }

        // ---------------------------------------------------------------
        // ucinewgame — reset state for a new game
        // ---------------------------------------------------------------
        else if (strcmp(line, "ucinewgame") == 0) {
            char new_fen[] = START_FEN;
            board = fen_to_bitboards(new_fen);
            update_occupancy(&board);
        }

        // ---------------------------------------------------------------
        // position [startpos|fen ...] [moves ...]
        // ---------------------------------------------------------------
        else if (strncmp(line, "position", 8) == 0) {
            board = parse_position(line);
        }

        // ---------------------------------------------------------------
        // go [depth|movetime|wtime|btime|...]
        // ---------------------------------------------------------------
        else if (strncmp(line, "go", 2) == 0) {
            GoParams params = parse_go(line);
            double time_limit = compute_time_limit(&params, board.player_turn == white_player);

            // Temporarily override time limit in find_best_move via a global
            // For now pass depth; time management is inside find_best_move already
            g_time_limit = time_limit;

            uint32_t best = find_best_move(&board, params.depth);

            char move_str[8];
            if (best) {
                uci_move_to_str(best, move_str);
            } else {
                strcpy(move_str, "0000");  // null move — no legal moves
            }

            printf("bestmove %s\n", move_str);
            fflush(stdout);
        }

        // ---------------------------------------------------------------
        // stop — stop searching (we search synchronously so just ignore)
        // ---------------------------------------------------------------
        else if (strcmp(line, "stop") == 0) {
            // No-op for now — search is synchronous
        }

        // ---------------------------------------------------------------
        // quit — exit cleanly
        // ---------------------------------------------------------------
        else if (strcmp(line, "quit") == 0) {
            break;
        }

        // ---------------------------------------------------------------
        // Non-standard: print board for debugging
        // ---------------------------------------------------------------
        else if (strcmp(line, "d") == 0) {
            print_board_string(join_board_string(board));
            fflush(stdout);
        }
    }
}