// Memory footprint test
// 1. Reports sizeof(struct Board) and sizeof(struct MoveList)
// 2. Allocates 1 million boards on the heap and measures actual RSS growth
//
// Compile: gcc -O2 -fopenmp -I. mem_test.c <objects> -o mem_test -lm
// Run:     ./mem_test

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "core/board.h"
#include "core/move_gen.h"
#include "core/core_util.h"
#include "core/attack.h"

// Read current process RSS (resident set size) in KB from /proc/self/status
static long get_rss_kb(void) {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return -1;
    char line[256];
    long rss = -1;
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "VmRSS: %ld kB", &rss) == 1) break;
    }
    fclose(f);
    return rss;
}

int main(void) {
    printf("sizeof(struct Board)    = %zu bytes\n", sizeof(struct Board));
    printf("sizeof(struct MoveList) = %zu bytes\n", sizeof(struct MoveList));
    printf("\n");

    long rss_before = get_rss_kb();
    printf("RSS before allocation: %ld KB\n", rss_before);

    const int N = 1000000;
    struct Board* boards = malloc(N * sizeof(struct Board));
    if (!boards) {
        printf("malloc failed for %d boards\n", N);
        return 1;
    }

    // Touch every board so pages are actually committed (not just reserved)
    init_attack_tables();
    char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    struct Board start = fen_to_bitboards(fen);
    update_occupancy(&start);

    for (int i = 0; i < N; i++) {
        boards[i] = start;  // full struct copy — same as engine does in search
    }

    long rss_after = get_rss_kb();
    printf("RSS after %d boards:  %ld KB\n", N, rss_after);
    printf("Delta:                %ld KB (%.2f MB)\n",
           rss_after - rss_before, (rss_after - rss_before) / 1024.0);
    printf("Expected (calc):      %.2f MB\n",
           (N * sizeof(struct Board)) / (1024.0 * 1024.0));

    free(boards);
    return 0;
}