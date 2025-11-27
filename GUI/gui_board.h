#ifndef GUI_BOARD_H
#define GUI_BOARD_H

#include <SDL3/SDL.h>

#include "../core/board.h"
#include "../core/movegen.h"

struct SDL_Renderer;

typedef struct {
    SDL_Texture* piece_textures[12];
} ChessTextures;

typedef struct {
    SDL_Texture* texture;
    uint64_t bitboards[12];
    int width, height;
} CachedPiecesTexture;

SDL_Texture* create_chessboard_texture(struct SDL_Renderer* renderer, int board_width, int board_height);
SDL_Texture* create_highlight_texture(struct SDL_Renderer* renderer, int board_width, int board_height, int square);
SDL_Texture* create_piece_highlight_texture(struct SDL_Renderer* renderer, int board_width, int board_height, struct MoveList* piece_moves);

ChessTextures load_pieces_textures(struct SDL_Renderer* renderer);
void destroy_pieces_textures(ChessTextures* textures);

CachedPiecesTexture init_cached_pieces(struct SDL_Renderer* renderer,
                                        const ChessTextures* textures,
                                        const struct Board* board,
                                        int width, int height);
void update_cached_pieces(struct SDL_Renderer* renderer,
                          CachedPiecesTexture* cache,
                          const ChessTextures* textures,
                          const struct Board* board);
void destroy_cached_pieces(CachedPiecesTexture* cache);


void mouse_click_to_board_pos(float mouse_x, float mouse_y, int window_width, int window_height, int* board_x, int* board_y);
struct MoveList selected_piece_moves(struct MoveList* moves_list, int board_index);

#endif 

