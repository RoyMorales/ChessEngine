// Evaluation and search

#include <omp.h>
#include <string.h>
#include <time.h>
#include "eval.h"
#include "../core/zobrist.h"

#define INF  1000000
#define MATE 100000

// Exposed so uci.c can override time per move
double g_time_limit = 5.0;

// ---------------------------------------------------------------------------
// Material values
// ---------------------------------------------------------------------------
static const int MATERIAL[12] = {
    0,    0,
    100, -100,
    320, -320,
    330, -330,
    500, -500,
    900, -900,
};

// Absolute material value by piece type (for MVV-LVA)
static const int PIECE_VALUE[12] = {
    20000, 20000,
    100,   100,
    320,   320,
    330,   330,
    500,   500,
    900,   900,
};

// ---------------------------------------------------------------------------
// Piece-square tables (white, a1=0, h8=63; black mirrors with sq^56)
// ---------------------------------------------------------------------------
static const int PST_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10,-20,-20, 10, 10,  5,
     5, -5,-10,  0,  0,-10, -5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
     0,  0,  0,  0,  0,  0,  0,  0,
};
static const int PST_KNIGHT[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -30,  5, 10, 15, 15, 10,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30,
   -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 10, 15, 15, 10,  0,-30,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50,
};
static const int PST_BISHOP[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  5,  0,  0,  0,  0,  5,-10,
   -10, 10, 10, 10, 10, 10, 10,-10,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0,  5, 10, 10,  5,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -20,-10,-10,-10,-10,-10,-10,-20,
};
static const int PST_ROOK[64] = {
     0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5, 10, 10, 10, 10, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0,
};
// Queen PST: strongly discourage early development.
// The queen scores best near home (d1/e1 area). Centre bonuses are small
// and only rewarded deep in the middlegame via search, not greedily by PST.
static const int PST_QUEEN[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  0,  0,  0,  0,  0,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20,
};
static const int PST_KING_MG[64] = {
    20, 30, 10,  0,  0, 10, 30, 20,
    20, 20,  0,  0,  0,  0, 20, 20,
   -10,-20,-20,-20,-20,-20,-20,-10,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
};

// ---------------------------------------------------------------------------
// Evaluation
// ---------------------------------------------------------------------------
// Penalise early queen development: if queen has left the back rank but
// the side still has undeveloped minor pieces (on their starting squares),
// add a penalty. This discourages Qd5 on move 1 without needing deep search.
static int queen_development_penalty(struct Board* board) {
    int penalty = 0;

    // White: queen off rank 1, count undeveloped knights/bishops
    uint64_t wq = board->bitboards[white_queen];
    if (wq && !((wq >> 0) & 0xFF)) {  // queen not on rank 1
        int undeveloped = 0;
        if (board->bitboards[white_knight] & (1ULL << 1))  undeveloped++; // b1
        if (board->bitboards[white_knight] & (1ULL << 6))  undeveloped++; // g1
        if (board->bitboards[white_bishop] & (1ULL << 2))  undeveloped++; // c1
        if (board->bitboards[white_bishop] & (1ULL << 5))  undeveloped++; // f1
        penalty -= undeveloped * 15;
    }

    // Black: queen off rank 8, count undeveloped knights/bishops
    uint64_t bq = board->bitboards[black_queen];
    if (bq && !((bq >> 56) & 0xFF)) {  // queen not on rank 8
        int undeveloped = 0;
        if (board->bitboards[black_knight] & (1ULL << 57)) undeveloped++; // b8
        if (board->bitboards[black_knight] & (1ULL << 62)) undeveloped++; // g8
        if (board->bitboards[black_bishop] & (1ULL << 58)) undeveloped++; // c8
        if (board->bitboards[black_bishop] & (1ULL << 61)) undeveloped++; // f8
        penalty += undeveloped * 15;  // good for white if black queen out early
    }

    return penalty;
}

int evaluation_material(struct Board* board) {
    int score = 0;
    for (int piece = 0; piece < 12; piece++) {
        uint64_t bb = board->bitboards[piece];
        bool is_black = (piece & 1);
        while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;
            // White PST uses sq directly; black mirrors vertically (sq^56)
            int pst_sq = is_black ? (sq ^ 56) : sq;
            int pst = 0;
            switch (piece >> 1) {
                case 0: pst = PST_KING_MG[pst_sq]; break;
                case 1: pst = PST_PAWN[pst_sq];    break;
                case 2: pst = PST_KNIGHT[pst_sq];  break;
                case 3: pst = PST_BISHOP[pst_sq];  break;
                case 4: pst = PST_ROOK[pst_sq];    break;
                case 5: pst = PST_QUEEN[pst_sq];   break;
            }
            // MATERIAL[] is already signed (+ve for white, -ve for black).
            // PST is always a bonus, so black pieces subtract it.
            if (is_black) {
                score += MATERIAL[piece] - pst;  // e.g. -900 - bonus
            } else {
                score += MATERIAL[piece] + pst;  // e.g. +900 + bonus
            }
        }
    }
    score += queen_development_penalty(board);
    return score;
}

static inline int side_score(struct Board* board) {
    int raw = evaluation_material(board);
    return (board->player_turn == white_player) ? raw : -raw;
}

// ---------------------------------------------------------------------------
// Transposition table
// ---------------------------------------------------------------------------
#define TT_SIZE (1 << 16)   // 64K entries (~1.5 MB)
#define TT_MASK (TT_SIZE - 1)

typedef enum { TT_EXACT, TT_LOWER, TT_UPPER } TTFlag;

typedef struct {
    uint64_t key;
    int      score;
    int      depth;
    TTFlag   flag;
    uint32_t best_move;
} TTEntry;

static TTEntry tt[TT_SIZE];

static void tt_clear(void) {
    memset(tt, 0, sizeof(tt));
}

static void tt_store(uint64_t key, int score, int depth, TTFlag flag, uint32_t move) {
    TTEntry* e = &tt[key & TT_MASK];
    // Always replace with deeper or equal depth
    if (depth >= e->depth) {
        e->key       = key;
        e->score     = score;
        e->depth     = depth;
        e->flag      = flag;
        e->best_move = move;
    }
}

static TTEntry* tt_probe(uint64_t key) {
    TTEntry* e = &tt[key & TT_MASK];
    return (e->key == key) ? e : NULL;
}

// ---------------------------------------------------------------------------
// Killer moves (2 per depth, up to MAX_DEPTH plies)
// ---------------------------------------------------------------------------
#define MAX_DEPTH 64
#define NUM_KILLERS 2

static uint32_t killers[MAX_DEPTH][NUM_KILLERS];

static void killers_clear(void) {
    memset(killers, 0, sizeof(killers));
}

static void killer_store(int depth, uint32_t move) {
    // Don't store captures as killers
    if ((move >> CAPTURE) & 1) return;
    if (killers[depth][0] == move) return;
    killers[depth][1] = killers[depth][0];
    killers[depth][0] = move;
}

// ---------------------------------------------------------------------------
// Move ordering
// MVV-LVA for captures, killers for quiet moves, then history
// ---------------------------------------------------------------------------
static int get_piece_on_square(struct Board* board, int sq) {
    uint64_t bit = 1ULL << sq;
    for (int p = 0; p < 12; p++)
        if (board->bitboards[p] & bit) return p;
    return -1;
}

static int move_score(struct Board* board, uint32_t move, int depth, uint32_t tt_move) {
    // TT move first
    if (move == tt_move) return 2000000;

    int to = (move >> 6) & 0x3F;

    // Queen promotion
    if (((move >> 12) & 0xF) == PROMOTION_QUEEN) return 1900000;

    // Captures: MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
    if ((move >> CAPTURE) & 1) {
        int from = move & 0x3F;
        int victim   = get_piece_on_square(board, to);
        int attacker = get_piece_on_square(board, from);
        int victim_val   = (victim   >= 0) ? PIECE_VALUE[victim]   : 100;
        int attacker_val = (attacker >= 0) ? PIECE_VALUE[attacker] : 100;
        return 1000000 + victim_val * 10 - attacker_val;
    }

    // En passant
    if ((move >> EN_PASSANT) & 1) return 1000000 + 100 * 10 - 100;

    // Killer moves
    if (depth < MAX_DEPTH) {
        if (move == killers[depth][0]) return 900000;
        if (move == killers[depth][1]) return 800000;
    }

    // Castling is generally good
    if ((move >> MOVE_CASTLING) & 1) return 100;

    return 0;
}

static void sort_moves(struct Board* board, struct MoveList* moves, int depth, uint32_t tt_move) {
    // Score all moves
    int scores[MAX_MOVES];
    for (int i = 0; i < moves->count; i++)
        scores[i] = move_score(board, moves->moves[i], depth, tt_move);

    // Insertion sort (lists are short, ~30 moves)
    for (int i = 1; i < moves->count; i++) {
        uint32_t key_move  = moves->moves[i];
        int      key_score = scores[i];
        int j = i - 1;
        while (j >= 0 && scores[j] < key_score) {
            moves->moves[j + 1] = moves->moves[j];
            scores[j + 1]       = scores[j];
            j--;
        }
        moves->moves[j + 1] = key_move;
        scores[j + 1]       = key_score;
    }
}

// ---------------------------------------------------------------------------
// Quiescence search — only searches captures to resolve tactical noise
// ---------------------------------------------------------------------------
static int quiescence(struct Board* board, int alpha, int beta) {
    int stand_pat = side_score(board);

    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    // Pseudo-legal captures only — much cheaper than generate_legal_moves
    struct MoveList moves = engine_search_moves(board);
    sort_moves(board, &moves, 0, 0);

    for (int i = 0; i < moves.count; i++) {
        uint32_t move = moves.moves[i];

        bool is_capture = (move >> CAPTURE) & 1;
        bool is_ep      = (move >> EN_PASSANT) & 1;
        bool is_promo_q = ((move >> 12) & 0xF) == PROMOTION_QUEEN;
        if (!is_capture && !is_ep && !is_promo_q) continue;

        struct Board child = *board;
        apply_move(&child, move);

        // Verify legality: our king must not be in check after the move
        // After apply_move, child.player_turn has flipped to the opponent.
        // Check if the side that just moved (board->player_turn) left their king in check.
        int ksq = get_king_square(&child, board->player_turn);
        // is_square_attacked by_white=true means white attacks the square
        // if mover was white, opponent (attacker) is black => by_white=false
        bool attacked_by_white = (board->player_turn != white_player);
        if (ksq >= 0 && is_square_attacked(&child, ksq, attacked_by_white)) continue;

        int score = -quiescence(&child, -beta, -alpha);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// ---------------------------------------------------------------------------
// Negamax with alpha-beta, TT, killers
// ---------------------------------------------------------------------------
int negamax(struct Board* board, int depth, int alpha, int beta) {
    uint64_t hash       = board_hash(board);
    int      orig_alpha = alpha;
    uint32_t tt_move    = 0;

    // TT lookup — copy entry locally to avoid mid-search races
    TTEntry* entry = tt_probe(hash);
    if (entry) {
        TTEntry e = *entry;
        tt_move = e.best_move;
        if (e.depth >= depth) {
            if (e.flag == TT_EXACT) return e.score;
            if (e.flag == TT_LOWER && e.score > alpha) alpha = e.score;
            if (e.flag == TT_UPPER && e.score < beta)  beta  = e.score;
            if (alpha >= beta) return e.score;
        }
    }

    if (depth == 0) return quiescence(board, alpha, beta);

    struct MoveList moves = generate_legal_moves(board);

    if (moves.count == 0) {
        int ksq = get_king_square(board, board->player_turn);
        if (ksq >= 0 && is_square_attacked(board, ksq, !board->player_turn))
            return -(MATE - (MAX_DEPTH - depth));
        return 0;  // stalemate
    }

    // Validate tt_move is actually legal before using it for ordering
    bool tt_move_valid = false;
    if (tt_move) {
        for (int i = 0; i < moves.count; i++) {
            if (moves.moves[i] == tt_move) { tt_move_valid = true; break; }
        }
    }

    sort_moves(board, &moves, depth, tt_move_valid ? tt_move : 0);

    uint32_t best_move  = 0;
    int      best_score = -INF;

    for (int i = 0; i < moves.count; i++) {
        struct Board child = *board;
        apply_move(&child, moves.moves[i]);

        int score = -negamax(&child, depth - 1, -beta, -alpha);

        if (score > best_score) {
            best_score = score;
            best_move  = moves.moves[i];
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) {
            killer_store(depth, moves.moves[i]);
            break;
        }
    }

    TTFlag flag = (best_score <= orig_alpha) ? TT_UPPER
                : (best_score >= beta)       ? TT_LOWER
                                             : TT_EXACT;
    tt_store(hash, best_score, depth, flag, best_move);

    return best_score;
}

// ---------------------------------------------------------------------------
// Iterative deepening with time limit — sequential (no parallel alpha-beta).
// Parallel root search breaks correctness because threads don't share the
// alpha window: a thread may promote a move that a properly-windowed search
// would prune. Sequential + TT + quiescence is already fast and correct.
// ---------------------------------------------------------------------------
uint32_t find_best_move(struct Board* board, int max_depth) {
    zobrist_init();
    killers_clear();

    struct MoveList moves = generate_legal_moves(board);
    if (moves.count == 0) return 0;
    if (moves.count == 1) return moves.moves[0];

    uint32_t best_move  = moves.moves[0];
    int      best_score = -INF;

    double time_limit = g_time_limit;
    double start      = omp_get_wtime();

    for (int depth = 1; depth <= max_depth; depth++) {
        killers_clear();
        sort_moves(board, &moves, depth, best_move);

        uint32_t depth_best_move  = moves.moves[0];
        int      depth_best_score = -INF;
        int      alpha            = -INF;

        for (int i = 0; i < moves.count; i++) {
            struct Board child = *board;
            apply_move(&child, moves.moves[i]);

            int score = -negamax(&child, depth - 1, -INF, -alpha);

            if (score > depth_best_score) {
                depth_best_score = score;
                depth_best_move  = moves.moves[i];
            }
            if (score > alpha) alpha = score;
        }

        best_move  = depth_best_move;
        best_score = depth_best_score;

        int from = best_move & 0x3F;
        int to   = (best_move >> 6) & 0x3F;
        double elapsed = omp_get_wtime() - start;
        // UCI info line — always goes to stdout so GUIs can display search progress
        printf("info depth %d score cp %d time %d pv %c%c%c%c\n",
               depth, best_score, (int)(elapsed * 1000),
               'a'+from%8, '1'+from/8, 'a'+to%8, '1'+to/8);
        fflush(stdout);

        if (omp_get_wtime() - start >= time_limit) break;
        if (best_score >  MATE - MAX_DEPTH)        break;
        if (best_score < -MATE + MAX_DEPTH)        break;
    }

    return best_move;
}