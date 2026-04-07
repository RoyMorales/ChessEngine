// PERFT - Move generation correctness testing

#include "util.h"
#include "../core/core_util.h"
#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Known correct node counts for standard starting position
// Source: https://www.chessprogramming.org/Perft_Results
static const uint64_t PERFT_EXPECTED[] = {
    1,                   // depth 0
    20,                  // depth 1
    400,                 // depth 2
    8902,                // depth 3
    197281,              // depth 4
    4865609,             // depth 5
    119060324,           // depth 6
    3195901860ULL,       // depth 7
    84998978956ULL,      // depth 8
    2439530234167ULL,    // depth 9
    69352859712417ULL,   // depth 10
};
#define PERFT_EXPECTED_MAX 10


void perft_count(struct Board* board, int depth, struct PerftStats* stats) {
    if (depth == 0) {
        stats->nodes++;
        return;
    }

    struct MoveList moves = generate_legal_moves(board);

    for (int i = 0; i < moves.count; i++) {
        uint32_t move = moves.moves[i];

        // Count stats only at depth 1 (leaf moves)
        // This matches the standard perft definition
        if (depth == 1) {
            stats->nodes++;
            if ((move >> CAPTURE) & 0x1)      stats->captures++;
            if ((move >> EN_PASSANT) & 0x1)    stats->ep++;
            if ((move >> MOVE_CASTLING) & 0x1) stats->castles++;
            if (((move >> 12) & 0xF) != 0)     stats->promotions++;
            continue;
        }

        struct Board backup = *board;
        apply_move(board, move);
        perft_count(board, depth - 1, stats);
        *board = backup;
    }
}


// Parallel perft: split root moves across threads, each with its own board copy.
// Threads never share mutable state so no locks needed. Scales with core count.
struct PerftStats perft_parallel(struct Board* root, int depth) {
    struct MoveList moves = generate_legal_moves(root);
    int n = moves.count;

    // Per-move stats array — one slot per root move, no contention
    struct PerftStats thread_stats[256] = {0};

    #pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < n; i++) {
        uint32_t move = moves.moves[i];

        if (depth == 1) {
            thread_stats[i].nodes++;
            if ((move >> CAPTURE) & 0x1)      thread_stats[i].captures++;
            if ((move >> EN_PASSANT) & 0x1)    thread_stats[i].ep++;
            if ((move >> MOVE_CASTLING) & 0x1) thread_stats[i].castles++;
            if (((move >> 12) & 0xF) != 0)     thread_stats[i].promotions++;
            continue;
        }

        struct Board board_copy = *root;  // Private copy — no sharing
        apply_move(&board_copy, move);
        perft_count(&board_copy, depth - 1, &thread_stats[i]);
    }

    // Merge results
    struct PerftStats total = {0};
    for (int i = 0; i < n; i++) {
        total.nodes      += thread_stats[i].nodes;
        total.captures   += thread_stats[i].captures;
        total.ep         += thread_stats[i].ep;
        total.castles    += thread_stats[i].castles;
        total.promotions += thread_stats[i].promotions;
    }
    return total;
}


// Divide perft: print node count per root move — use to compare against Stockfish
void perft_divide(struct Board* board, int depth) {
    struct MoveList moves = generate_legal_moves(board);
    uint64_t total = 0;

    printf("%-8s  %s\n", "Move", "Nodes");
    printf("------------------\n");

    for (int i = 0; i < moves.count; i++) {
        uint32_t move = moves.moves[i];

        char move_str[6];
        int from = move & 0x3F;
        int to   = (move >> 6) & 0x3F;
        move_str[0] = 'a' + (from % 8);
        move_str[1] = '1' + (from / 8);
        move_str[2] = 'a' + (to % 8);
        move_str[3] = '1' + (to / 8);

        int promo = (move >> 12) & 0xF;
        if (promo) {
            const char promo_chars[] = {0, 'q', 'r', 'b', 'n'};
            move_str[4] = (promo < 5) ? promo_chars[promo] : '?';
            move_str[5] = '\0';
        } else {
            move_str[4] = '\0';
        }

        struct PerftStats sub = {0};
        struct Board backup = *board;
        apply_move(board, move);
        if (depth == 1) {
            sub.nodes = 1;
        } else {
            perft_count(board, depth - 1, &sub);
        }
        *board = backup;

        printf("%-8s  %lu\n", move_str, sub.nodes);
        total += sub.nodes;
    }

    printf("------------------\n");
    printf("Moves: %d   Total nodes: %lu\n", moves.count, total);
}


int main(int argc, char** argv) {
    char fen_setup[256] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    int depth = 5;
    int do_divide  = 0;
    int do_verify  = 0;
    int do_parallel = 1;  // parallel by default

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--divide") == 0) {
            do_divide = 1;
        } else if (strcmp(argv[i], "--verify") == 0) {
            do_verify = 1;
        } else if (strcmp(argv[i], "--fen") == 0 && i + 1 < argc) {
            strncpy(fen_setup, argv[++i], sizeof(fen_setup) - 1);
        } else if (strcmp(argv[i], "--single") == 0) {
            do_parallel = 0;
        } else {
            int n = atoi(argv[i]);
            if (n > 0) depth = n;
        }
    }

    struct Board board = fen_to_bitboards(fen_setup);
    update_occupancy(&board);
    init_attack_tables();

    printf("Depth:   %d\n", depth);
    printf("Threads: %d\n", do_parallel ? omp_get_max_threads() : 1);
    printf("----------------------------\n");

    if (do_divide) {
        perft_divide(&board, depth);
        return 0;
    }

    double start = omp_get_wtime();

    struct PerftStats stats;
    if (do_parallel) {
        stats = perft_parallel(&board, depth);
    } else {
        stats = (struct PerftStats){0};
        perft_count(&board, depth, &stats);
    }

    double exe_time = omp_get_wtime() - start;
    double mnps = (exe_time > 0) ? (stats.nodes / 1e6) / exe_time : 0.0;

    printf("PERFT(%d)    = %lu\n", depth, stats.nodes);
    printf("Captures    = %lu\n", stats.captures);
    printf("En Passant  = %lu\n", stats.ep);
    printf("Castles     = %lu\n", stats.castles);
    printf("Promotions  = %lu\n", stats.promotions);
    printf("Time        = %.3f s\n", exe_time);
    printf("Speed       = %.2f Mn/s\n", mnps);

    if (do_verify && depth <= PERFT_EXPECTED_MAX) {
        uint64_t expected = PERFT_EXPECTED[depth];
        if (stats.nodes == expected) {
            printf("PASS  (expected %lu)\n", expected);
        } else {
            printf("FAIL  expected %lu, got %lu (diff %ld)\n",
                   expected, stats.nodes, (long)(stats.nodes - expected));
        }
    }

    printf("----------------------------\n");
    return 0;
}