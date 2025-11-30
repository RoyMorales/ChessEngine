// Chessboard made with SDL3

#include <stdio.h>
#include <SDL3_image/SDL_image.h>

#include "gui_board.h"

void draw_chessboard(SDL_Renderer* renderer, int width, int height) {
  int tile_size = (width < height ? width : height) / 8;
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      SDL_FRect rect = {(float)col * tile_size,
                        (float) row * tile_size,
                        (float) tile_size,
                        (float) tile_size};
      if ((row + col) % 2 == 0) {
        // ToDO! Move colors to config file
        SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255); // Light color
      } else {
        // ToDO! Move colors to config file
        SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255); // Dark color
      }
      SDL_RenderFillRect(renderer, &rect);
    }
  }
}

SDL_Texture* create_chessboard_texture(SDL_Renderer* renderer, int board_width, int board_height) {
  SDL_Texture* board_texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET,
    board_width, board_height);

  SDL_SetRenderTarget(renderer, board_texture);
  draw_chessboard(renderer, board_width, board_height);
  SDL_SetRenderTarget(renderer, NULL);
  return board_texture;
}

void draw_hightlight_square(SDL_Renderer* renderer, int width, int height, int square) {
  int tile_size = (width < height ? width : height) / 8;
  int row = 7 - (square / 8);
  int col = square % 8;

  SDL_FRect rect = {
    (float)(col * tile_size),
    (float)(row * tile_size),
    (float)tile_size,
    (float)tile_size
  };

  // ToDO! Move highlight color to config file
  SDL_SetRenderDrawColor(renderer, 210, 4, 45, 255); // Highlight color no transparency
  SDL_RenderFillRect(renderer, &rect);
}

SDL_Texture* create_highlight_texture(SDL_Renderer* renderer, int board_width, int board_height, int square) {
  SDL_Texture* highlight_texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET,
    board_width, board_height);

  SDL_SetTextureBlendMode(highlight_texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, highlight_texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  draw_hightlight_square(renderer, board_width, board_height, square);
  SDL_SetRenderTarget(renderer, NULL);
  return highlight_texture;
}

void draw_piece_highlight_squares(SDL_Renderer* renderer, int width, int height, struct MoveList* moves_list) {
  int tile_size = (width < height ? width : height) / 8;
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128); // Semi-transparent green

  for (int i = 0; i < moves_list->count; i++) {
    uint32_t move = moves_list->moves[i];
    int to_square = (move >> 6) & 0x3F;

    int row = 7 - (to_square / 8);
    int col = to_square % 8;

    SDL_FRect rect = {
      (float)(col * tile_size),
      (float)(row * tile_size),
      (float)tile_size,
      (float)tile_size
    };
    SDL_RenderFillRect(renderer, &rect);
  }
}

SDL_Texture* create_piece_highlight_texture(SDL_Renderer* renderer, int board_width, int board_height, struct MoveList* moves_list) {
  SDL_Texture* highlight_texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET,
    board_width, board_height);

  SDL_SetTextureBlendMode(highlight_texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, highlight_texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  draw_piece_highlight_squares(renderer, board_width, board_height, moves_list);
  SDL_SetRenderTarget(renderer, NULL);
  return highlight_texture;
}

// ToDO! Move texture path to config file
struct ChessTextures load_pieces_textures(SDL_Renderer* renderer) {
  struct ChessTextures textures = {0};

  const char* texture_files[12] = {
    "resources/pieces/king_white.png",
    "resources/pieces/king_black.png",
    "resources/pieces/pawn_white.png",
    "resources/pieces/pawn_black.png",
    "resources/pieces/knight_white.png",
    "resources/pieces/knight_black.png",
    "resources/pieces/bishop_white.png",
    "resources/pieces/bishop_black.png",
    "resources/pieces/rook_white.png",
    "resources/pieces/rook_black.png",
    "resources/pieces/queen_white.png",
    "resources/pieces/queen_black.png"
  };

  for (int i = 0; i < 12; i++) {
    textures.piece_textures[i] = IMG_LoadTexture(renderer, texture_files[i]);
    if (!textures.piece_textures[i]) {
      fprintf(stderr, "Failed to load texture %s: %s\n",
              texture_files[i], SDL_GetError());
    }
  }
  return textures;
}

void destroy_pieces_textures(struct ChessTextures* textures) {
  for (int i = 0; i < 12; i++) {
    if (textures->piece_textures[i]) {
      SDL_DestroyTexture(textures->piece_textures[i]);
      textures->piece_textures[i] = NULL;
    }
  }
}

static inline void render_piece(SDL_Renderer* renderer, SDL_Texture* piece_tex,
                                int tile_size, float scale, int square) {
  int row = 7 - (square / 8);
  int col = square % 8;
  float offset = tile_size * (1.0f - scale) * 0.5f;

  SDL_FRect dest_rect = {
    col * tile_size + offset,
    row * tile_size + offset,
    tile_size * scale,
    tile_size * scale
  };
  SDL_RenderTexture(renderer, piece_tex, NULL, &dest_rect);
}

struct CachedPiecesTexture init_cached_pieces(SDL_Renderer* renderer,
                                       const struct ChessTextures* textures,
                                       const struct Board* board,
                                       int width, int height) {
  struct CachedPiecesTexture cache = {0};
  cache.width = width;
  cache.height = height;

  cache.texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    width, height);

  SDL_SetTextureBlendMode(cache.texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, cache.texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  int tile_size = (width < height ? width : height) / 8;
  float scale = 0.7f;

  for (int piece = 0; piece < 12; piece++) {
    uint64_t bits = board->bitboards[piece];
    while (bits) {
      int square = __builtin_ctzll(bits);
      bits &= bits - 1;
      render_piece(renderer, textures->piece_textures[piece], tile_size, scale, square);
    }
    cache.bitboards[piece] = board->bitboards[piece];
  }
  SDL_SetRenderTarget(renderer, NULL);
  return cache;
}

void destroy_cached_pieces(struct CachedPiecesTexture* cache) {
  if (cache->texture) SDL_DestroyTexture(cache->texture);
  cache->texture = NULL;
}
