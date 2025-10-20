// Chessboard made with SDL3

#include <stdio.h>
#include <SDL3/SDL.h>
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
        SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255); // Light color
      } else {
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

// ToDO! Move texture path to config file
ChessTextures load_pieces_textures(SDL_Renderer* renderer) {
  ChessTextures textures = {0};

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

void destroy_pieces_textures(ChessTextures* textures) {
  for (int i = 0; i < 12; i++) {
    if (textures->piece_textures[i]) {
      SDL_DestroyTexture(textures->piece_textures[i]);
      textures->piece_textures[i] = NULL;
      }
  }
}

void create_pieces_texture(SDL_Renderer* renderer, ChessTextures* textures, int board_width, int board_height, struct Board* board) {
  int tile_size = (board_width < board_height ? board_width : board_height) / 8;
  float piece_scale = 0.7f;

  for (int piece_type = 0; piece_type < 12; piece_type++) {
    uint64_t bitboard = board->bitboards[piece_type];
    for (int square = 0; square < 64; square++) {
      if (bitboard & (1ULL << square)) {
        int row = 7 - (square / 8);
        int col = square % 8;

        SDL_FRect dest_rect = {
          (float)((col * tile_size) + (tile_size * (1 - piece_scale) / 2)),
          (float)(row * tile_size) + (tile_size * (1 - piece_scale) / 2),
          (float)tile_size - tile_size * (1 - piece_scale),
          (float)tile_size - tile_size * (1 - piece_scale)
        };

        SDL_RenderTexture(renderer, textures->piece_textures[piece_type],
                          NULL, &dest_rect);
        }
    }
  }
}




