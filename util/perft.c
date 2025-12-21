// PERFT

#include "util.h"
#include <time.h>
#include <stdlib.h>


void perft_count(struct Board* board, int depth, struct PerftStats* stats) {
    if (depth == 0) {
        stats->nodes += 1;
        return;
    }

    //if (depth == 1) { 
    //    printf("-----------------------------\n");
    //    printf("Depth %d: Generating moves...\n\n", depth);
    //}
    

    struct MoveList moves = generate_legal_moves(board);

    for (int i = 0; i < moves.count; i++) {
        uint32_t move = moves.moves[i];

        //print_move(move);

        struct Board backup = *board;

        apply_move(board, move);

        // Update stats for this move
        if ((move >> CAPTURE) & 0x1) stats->captures++;
        if ((move >> EN_PASSANT) & 0x1) stats->ep++;
        if ((move >> MOVE_CASTLING) & 0x1) stats->castles++;
        if (((move >> 12) & 0xF) != 0) stats->promotions++;
        
        perft_count(board, depth - 1, stats);

        *board = backup;
    }
}


int main(int argc, char** argv) {
    char fen_setup[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
    //char fen_setup[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
    //char fen_setup[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ";
    //char fen_setup[] = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ";

    struct Board board = fen_to_bitboards(fen_setup);
    update_occupancy(&board);
    init_attack_tables();

    int depth = 4;
    if (argc > 1) depth = atoi(argv[1]);

    struct PerftStats stats = {0};

    clock_t start = clock();
    perft_count(&board, depth, &stats);
    clock_t end = clock();

    double exe_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("----------------------------\n");
    printf("PERFT(%d) = %lu\n", depth, stats.nodes);
    printf("Captures: %lu\n", stats.captures);
    printf("En Passant: %lu\n", stats.ep);
    printf("Castles: %lu\n", stats.castles);
    printf("Promotions: %lu\n", stats.promotions);
    printf("Exe Time: %.3f s \n", exe_time);
    printf("----------------------------\n");

    return 0;
}

