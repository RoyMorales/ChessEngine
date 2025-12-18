#ifndef GUI_BOARD_H
#define GUI_BOARD_H

#include <SDL3/SDL.h>

#include "../core/board.h"
#include "../core/move_gen.h"
#include "../util/util.h"

struct SDL_Renderer;
struct BoardStateUI;
struct RenderContext;
struct Config;
struct SideData;

struct ChessTextures {
    SDL_Texture* piece_textures[12];
};

struct CachedPiecesTexture{
    SDL_Texture* texture;
    uint64_t bitboards[12];
    int width, height;
};

struct BoardStateUI{
    bool need_redraw;
    bool selected_movable_piece;
    bool piece_moved;
    int board_x;
    int board_y;
    int board_index;
};

struct RenderContext {
    SDL_Renderer* renderer;
    SDL_Texture* board_texture;
    SDL_Texture* highlight_texture;
    SDL_Texture* highlight_piece_texture;
    struct CachedPiecesTexture pieces_cache;
};

struct SideData {
    uint64_t opponent_pieces;
    uint64_t player_pieces;
};

SDL_Texture* create_chessboard_texture(struct SDL_Renderer* renderer, int board_width, int board_height);
SDL_Texture* create_highlight_texture(struct SDL_Renderer* renderer, int board_width, int board_height, int square);
SDL_Texture* create_piece_highlight_texture(struct SDL_Renderer* renderer, int board_width, int board_height, struct MoveList* piece_moves);

struct ChessTextures load_pieces_textures(struct SDL_Renderer* renderer);
void destroy_pieces_textures(struct ChessTextures* textures);

struct CachedPiecesTexture init_cached_pieces(struct SDL_Renderer* renderer,
                                        const struct ChessTextures* textures,
                                        const struct Board* board,
                                        int width, int height);
void destroy_cached_pieces(struct CachedPiecesTexture* cache);
void main_switch_event(SDL_Event* event, bool* running, uint32_t* move, struct MoveList* move_list, struct MoveList* move_list_piece,
                    struct BoardStateUI* board_state_ui, struct RenderContext* render_context, struct Config* config, struct SideData* side_data);
void mouse_click_to_board_pos(float mouse_x, float mouse_y, int window_width, int window_height, int* board_x, int* board_y);
struct MoveList* select_pieces_moves(struct MoveList* move_list, int board_index);
uint32_t select_move_from_list(struct MoveList* move_list, int board_index);
void fast_update_piece_cache(struct RenderContext* render_context,
                             const struct ChessTextures* textures,
                             const struct Board* board,
                             uint32_t move,
                             int window_width,
                             int window_height);

#endif
