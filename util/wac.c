// WAC (Win At Chess) test suite runner
// Reads an EPD file, runs find_best_move on each position,
// compares against the known best move, and reports a score.
//
// EPD format: <FEN w/o move counts> bm <move>; id "<id>";
// Example:    2rr3k/... w - - bm Nxg6; id "WAC.001";
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
 
#include "util.h"
#include "../core/core_util.h"
#include "../core/board.h"
#include "../core/attack.h"
#include "../core/move_gen.h"
#include "../core/move_filter.h"
#include "../core/move_apply.h"
#include "../core/zobrist.h"
#include "../engine/eval.h"
 
#define MAX_LINE 512
#define DEFAULT_EPD "util/wac.epd"
#define DEFAULT_DEPTH 5
#define DEFAULT_TIME  10.0   // seconds per position
 
// ---------------------------------------------------------------------------
// Convert a move (uint32_t) to algebraic notation for comparison.
// We use coordinate notation (e2e4) and also try to match SAN (Nxg6).
// ---------------------------------------------------------------------------
static void move_to_coord(uint32_t move, char* out) {
    int from = move & 0x3F;
    int to   = (move >> 6) & 0x3F;
    out[0] = 'a' + from % 8;
    out[1] = '1' + from / 8;
    out[2] = 'a' + to % 8;
    out[3] = '1' + to / 8;
    int promo = (move >> 12) & 0xF;
    if (promo) {
        const char pc[] = {0, 'q', 'r', 'b', 'n'};
        out[4] = (promo < 5) ? pc[promo] : '?';
        out[5] = '\0';
    } else {
        out[4] = '\0';
    }
}
 
// Convert a move to SAN-like notation for matching against EPD bm field.
// We produce a simplified form: piece letter + capture 'x' + destination.
// Handles the most common cases — promotions, castling, pawn moves.
static void move_to_san(struct Board* board, uint32_t move, char* out) {
    int from     = move & 0x3F;
    int to       = (move >> 6) & 0x3F;
    bool capture = (move >> CAPTURE) & 1;
    bool ep      = (move >> EN_PASSANT) & 1;
    bool castle  = (move >> MOVE_CASTLING) & 1;
    int  promo   = (move >> 12) & 0xF;
 
    if (castle) {
        strcpy(out, (to > from) ? "O-O" : "O-O-O");
        return;
    }
 
    // Find piece on from square
    const char piece_chars[] = "KKPPNNBBRRQQ";  // 12 entries, one per piece index
    int piece = -1;
    for (int p = 0; p < 12; p++) {
        if (board->bitboards[p] & (1ULL << from)) { piece = p; break; }
    }
 
    char buf[8];
    int idx = 0;
 
    if (piece >= 0 && piece >> 1 != 1) {
        // Non-pawn: add piece letter
        buf[idx++] = piece_chars[piece];
    } else if (capture || ep) {
        // Pawn capture: add file of origin
        buf[idx++] = 'a' + from % 8;
    }
 
    if (capture || ep) buf[idx++] = 'x';
 
    buf[idx++] = 'a' + to % 8;
    buf[idx++] = '1' + to / 8;
 
    if (promo) {
        buf[idx++] = '=';
        const char pc[] = {0, 'Q', 'R', 'B', 'N'};
        buf[idx++] = (promo < 5) ? pc[promo] : '?';
    }
 
    buf[idx] = '\0';
    strcpy(out, buf);
}
 
// ---------------------------------------------------------------------------
// Parse the bm field from an EPD line — may contain multiple moves.
// Returns number of best moves found.
// ---------------------------------------------------------------------------
static int parse_bm(const char* line, char bm_moves[][16], int max_bm) {
    const char* p = strstr(line, " bm ");
    if (!p) return 0;
    p += 4;
 
    int count = 0;
    while (*p && *p != ';' && count < max_bm) {
        // Skip spaces
        while (*p == ' ') p++;
        if (!*p || *p == ';') break;
 
        int i = 0;
        while (*p && *p != ' ' && *p != ';' && i < 15)
            bm_moves[count][i++] = *p++;
        bm_moves[count][i] = '\0';
 
        // Strip check (+) and checkmate (#) suffixes — we don't generate these
        int len = strlen(bm_moves[count]);
        while (len > 0 && (bm_moves[count][len-1] == '+' || bm_moves[count][len-1] == '#'))
            bm_moves[count][--len] = '\0';
 
        if (i > 0) count++;
    }
    return count;
}
 
// ---------------------------------------------------------------------------
// Parse the id field from an EPD line
// ---------------------------------------------------------------------------
static void parse_id(const char* line, char* id_out) {
    const char* p = strstr(line, " id \"");
    if (!p) { strcpy(id_out, "?"); return; }
    p += 5;
    int i = 0;
    while (*p && *p != '"' && i < 31) id_out[i++] = *p++;
    id_out[i] = '\0';
}
 
// ---------------------------------------------------------------------------
// Build a full FEN from an EPD line (EPD omits move counters)
// ---------------------------------------------------------------------------
static void epd_to_fen(const char* line, char* fen_out) {
    // EPD: <pieces> <turn> <castling> <ep>  (no half/full move)
    // We append " 0 1" to make a valid FEN
    strncpy(fen_out, line, 255);
    fen_out[255] = '\0';
 
    // Truncate at " bm"
    char* bm = strstr(fen_out, " bm");
    if (bm) *bm = '\0';
 
    strncat(fen_out, " 0 1", 255 - strlen(fen_out));
}
 
// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
    const char* epd_file  = DEFAULT_EPD;
    int         depth     = DEFAULT_DEPTH;
    double      time_limit = DEFAULT_TIME;
    int         verbose   = 0;
 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i+1 < argc) depth      = atoi(argv[++i]);
        else if (strcmp(argv[i], "-t") == 0 && i+1 < argc) time_limit = atof(argv[++i]);
        else if (strcmp(argv[i], "-v") == 0) verbose = 1;
        else if (strcmp(argv[i], "--epd") == 0 && i+1 < argc) epd_file = argv[++i];
        else epd_file = argv[i];
    }
 
    FILE* f = fopen(epd_file, "r");
    if (!f) {
        fprintf(stderr, "Cannot open EPD file: %s\n", epd_file);
        return 1;
    }
 
    init_attack_tables();
    zobrist_init();
 
    int total = 0, correct = 0;
    char line[MAX_LINE];
    double total_time = 0.0;
 
    printf("WAC Test Suite — depth=%d time=%.1fs file=%s\n", depth, time_limit, epd_file);
    printf("%-12s %-8s %-8s %-6s %s\n", "ID", "Engine", "Best", "Result", "Time");
    printf("----------------------------------------------\n");
 
    while (fgets(line, MAX_LINE, f)) {
        // Strip newline
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 10) continue;
 
        char id[32];
        parse_id(line, id);
 
        char bm[8][16];
        int  bm_count = parse_bm(line, bm, 8);
        if (bm_count == 0) continue;
 
        char fen[256];
        epd_to_fen(line, fen);
 
        struct Board board = fen_to_bitboards(fen);
        update_occupancy(&board);
 
        // Sanity check — if legal move count is 0 the position parsed wrong
        struct MoveList sanity = generate_legal_moves(&board);
        if (sanity.count == 0) {
            printf("%-12s %-8s %-8s SKIP  (0 legal moves — bad FEN?)\n", id, "-", bm[0]);
            total++;
            fflush(stdout);
            continue;
        }
 
        double t0 = omp_get_wtime();
        uint32_t best = find_best_move(&board, depth);
        double elapsed = omp_get_wtime() - t0;
        total_time += elapsed;
 
        if (best == 0) {
            printf("%-12s %-8s %-8s FAIL  %.2fs (no move returned)\n",
                   id, "none", bm[0], elapsed);
            total++;
            fflush(stdout);
            continue;
        }
 
        char coord[8], san[16];
        move_to_coord(best, coord);
        move_to_san(&board, best, san);
 
        // Check if engine move matches any accepted best move
        bool hit = false;
        for (int i = 0; i < bm_count; i++) {
            if (strcmp(coord, bm[i]) == 0 || strcmp(san, bm[i]) == 0) {
                hit = true; break;
            }
        }
 
        total++;
        if (hit) correct++;
 
        if (verbose || !hit) {
            printf("%-12s %-8s %-8s %-6s %.2fs\n",
                   id, san, bm[0],
                   hit ? "PASS" : "FAIL",
                   elapsed);
        } else {
            printf("%-12s %-8s %-8s PASS  %.2fs\n", id, san, bm[0], elapsed);
        }
        fflush(stdout);
    }
 
    fclose(f);
 
    printf("----------------------------------------------\n");
    printf("Score:  %d / %d  (%.1f%%)\n", correct, total, 100.0 * correct / (total ? total : 1));
    printf("Time:   %.1f s total, %.2f s/position\n", total_time, total_time / (total ? total : 1));
    return 0;
}
