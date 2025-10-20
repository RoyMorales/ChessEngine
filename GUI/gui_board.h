#ifndef GUI_BOARD_H
#define GUI_BOARD_H

#include "../core/logic.h"

struct SDL_Renderer;

typedef struct {
    SDL_Texture* piece_textures[12];
} ChessTextures;

SDL_Texture* create_chessboard_texture(struct SDL_Renderer* renderer, int board_width, int board_height);
ChessTextures load_pieces_textures(struct SDL_Renderer* renderer);
void destroy_pieces_textures(ChessTextures* textures);
void create_pieces_texture(struct SDL_Renderer* renderer, ChessTextures* textures, int board_width, int board_height, struct Board* board);

void mouse_click_to_board_pos(float mouse_x, float mouse_y, int window_width, int window_height, int* board_x, int* board_y);

#endif 

