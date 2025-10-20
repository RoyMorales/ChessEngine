// Main Chess

#include <stdio.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#include "util/util.h"
#include "GUI/gui_board.h"
#include "core/logic.h"

#define TARGET_FPS 60
#define FRAME_TIME_MS (1000.0 / TARGET_FPS)
#define FPS_UPDATE_INTERVAL 5

int main(void) {
  Config config;
  if (config_reader("settings.cfg", &config) != 0) {
    fprintf(stderr, "Failed to read config file\n");
    SDL_Quit();
    return 1;
  }
  
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 2;
  }

  SDL_Window *window = SDL_CreateWindow("SDL3 Test",
                                        config.window_height,
                                        config.window_height,
                                        config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
  if (!window) {
      fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
      SDL_Quit();
      return 3;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  if (!renderer) {
      fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
      SDL_DestroyWindow(window);
      SDL_Quit();
      return 4;
  }

  char fen_setup[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  struct Board game_board = fen_to_bitboards(fen_setup); 

  SDL_Event event;
  SDL_Texture* board_texture = create_chessboard_texture(renderer, config.window_width, config.window_height);
  ChessTextures textures = load_pieces_textures(renderer);

  uint64_t frame_start, frame_end, frame_time;
  FPSCounter fps_counter;
  fps_init(&fps_counter);

  int board_x;
  int board_y;
  
  bool running = true;
  while (running) {
    frame_start = SDL_GetTicks();

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;

        case SDL_EVENT_KEY_DOWN:
          if (event.key.key == SDLK_ESCAPE) {
            running = false;
          }
          break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
          mouse_click_to_board_pos(event.button.x, event.button.y, config.window_width, config.window_height, &board_x, &board_y);
          printf("Board Position: (%d, %d)\n", board_x, board_y);
          break;

        default:
            break;
        }
      }
    
    SDL_RenderTexture(renderer, board_texture, NULL, NULL);  
    create_pieces_texture(renderer, &textures, config.window_width, config.window_height, &game_board);
    SDL_RenderPresent(renderer);
      
    frame_end = SDL_GetTicks();
    frame_time = frame_end - frame_start;

    if (frame_time < FRAME_TIME_MS) {
      SDL_Delay(FRAME_TIME_MS - frame_time);
    }
    fps_update_terminal(&fps_counter, FPS_UPDATE_INTERVAL);
  }


  destroy_pieces_textures(&textures);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}


