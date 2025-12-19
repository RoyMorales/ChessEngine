// PERFT

#include "util.h"
#include <time.h>
#include <stdlib.h>


void print_move_perft(uint32_t move) {
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    int flags = move >> 12;

    // Basic move notation
    char move_str[16];
    snprintf(move_str, sizeof(move_str), "%c%d -> %c%d",
             'a' + (from % 8), 1 + (from / 8),
             'a' + (to % 8), 1 + (to / 8));

    // Add special flags
    if (flags & (1 << CAPTURE))       printf("%s x", move_str);
    else                               printf("%s", move_str);

    if (flags & (1 << MOVE_CASTLING)) printf(" (castle)");
    if (flags & (1 << DOUBLE_PUSH))   printf(" (2x)");
    if (flags & (1 << EN_PASSANT))    printf(" (ep)");

    // Promotion
    if (flags & PROMOTION_QUEEN)      printf("=Q");
    if (flags & PROMOTION_ROOK)       printf("=R");
    if (flags & PROMOTION_BISHOP)     printf("=B");
    if (flags & PROMOTION_KNIGHT)     printf("=N");

    printf("\n");
}


uint64_t perft(struct Board* board, int depth) {
    if (depth == 0) return 1;

    uint64_t nodes = 0;

    struct MoveList moves = generate_legal_moves(board);

    for (int i = 0; i < moves.count; i++) {
        uint32_t move = moves.moves[i];

        struct Board backup = *board;   // TEMP copy (slow but correct)

        apply_move(board, move);
        nodes += perft(board, depth - 1);

        *board = backup;                // restore board
    }

    return nodes;
}


uint64_t perft_divide(struct Board* board, int depth) {
    struct MoveList moves = generate_legal_moves(board);
    uint64_t total_nodes = 0;

    for (int i = 0; i < moves.count; i++) {
        struct Board board_copy = *board;  // Make a copy
        apply_move(&board_copy, moves.moves[i]);

        uint64_t nodes = 0;
        if (depth - 1 > 0) {
            nodes = perft(&board_copy, depth - 1);
        } else {
            nodes = 1;  // Leaf node
        }

        total_nodes += nodes;
        
        print_move_perft(moves.moves[i]);
        printf("Nodes: %lu\n", nodes);
    }
    return total_nodes;
}


int main(int argc, char** argv) {
    char fen_setup[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    struct Board board = fen_to_bitboards(fen_setup);
    update_occupancy(&board);
    init_attack_tables();

    int depth = 4;
    if (argc > 1) {
        depth = atoi(argv[1]);
    }

    clock_t start = clock();
    uint64_t nodes = perft_divide(&board, depth);
    clock_t end = clock();

    double exe_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("----------------------------\n");
    printf("PERFT(%d) = %lu\n", depth, nodes);
    printf("Exe Time: %.3f s \n", exe_time);
    printf("----------------------------\n");

    return 0;
}
