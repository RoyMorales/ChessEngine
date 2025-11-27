// Main Chess

#include <stdio.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#include "util/util.h"
#include "GUI/gui_board.h"
#include "core/core_util.h"
#include "core/board.h"
#include "core/movegen.h"
#include "core/attack.h"

#define FPS_UPDATE_INTERVAL 10
#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)


int main(void) {
  printf("\n----------------------------\n");
  printf("         Chess Engine      \n");
  printf("----------------------------\n");
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

  SDL_PropertiesID renderer_props = SDL_CreateProperties();
  SDL_SetPointerProperty(renderer_props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
  SDL_SetNumberProperty(renderer_props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1);
  SDL_SetStringProperty(renderer_props, SDL_PROP_RENDERER_CREATE_NAME_STRING, "opengl");

  SDL_Renderer* renderer = SDL_CreateRendererWithProperties(renderer_props);
  printf("Renderer backend: %s\n", SDL_GetRendererName(renderer));

  SDL_DestroyProperties(renderer_props);

  if (!renderer) {
    fprintf(stderr, "SDL_CreateRendererWithProperties failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 4;
  }

  SDL_PropertiesID rprops = SDL_GetRendererProperties(renderer);
  bool vsync_enabled = SDL_GetNumberProperty(rprops, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 0);
  printf("VSync enabled: %s\n", vsync_enabled ? "true" : "false");
  printf("----------------------------\n");


  char fen_setup[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  struct Board game_board = fen_to_bitboards(fen_setup); 
  update_occupancy(&game_board);
  init_attack_tables();

  SDL_Event event; 
  ChessTextures textures = load_pieces_textures(renderer);
  SDL_Texture* board_texture = create_chessboard_texture(renderer, config.window_width, config.window_height);
  SDL_Texture* highlight_texture = create_highlight_texture(renderer, config.window_width, config.window_height, -1);
  SDL_Texture* highlight_piece_texture = NULL;
  CachedPiecesTexture pieces_cache = init_cached_pieces(renderer, &textures, &game_board, config.window_width, config.window_height);

  // FPS management variables
  uint64_t frame_start, frame_end, frame_time;
  FPSCounter fps_counter;
  fps_init(&fps_counter);

  // Board position variables
  int board_x;
  int board_y;
  int board_index;
  
  // Main loop variables
  bool need_redraw = true;
  bool running = true;
  bool generate_moves = true;
  struct MoveList moves;
  struct MoveList piece_move_list;

  // ToDO! Implement player choice
  char player = white_player;
  char opponent = (player == white_player) ? black_player : white_player;
  uint64_t player_pieces = (player == white_player) ? game_board.white_occupied : game_board.black_occupied; 
  uint64_t opponent_pieces = (opponent == white_player) ? game_board.white_occupied : game_board.black_occupied;

  while (running) {
    frame_start = SDL_GetTicks();

    if (generate_moves) {
      moves = genrate_board_moves(&game_board);
      print_move_list(&moves);
      generate_moves = false;
    }

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
          board_index = board_y  * 8 + board_x;
          printf("Board Position: (%d, %d) ", board_x, board_y);
          printf("| Square Index: %d\n", board_index);

          if(player_pieces & (1ULL << board_index)) {
            // Highlight selected piece
            highlight_texture = create_highlight_texture(renderer, config.window_width, config.window_height, board_index);
            
            // Hightlight possible moves
            piece_move_list = selected_piece_moves(&moves, board_index);
            print_move_list(&piece_move_list);
            highlight_piece_texture = create_piece_highlight_texture(renderer, config.window_width, config.window_height, &piece_move_list);
            need_redraw = true;  

          } else {
            highlight_texture = create_highlight_texture(renderer, config.window_width, config.window_height, -1);
            need_redraw = true;
          }


          break;

        default:
            break;
        }
      }
    
    if (need_redraw) {
      SDL_RenderTexture(renderer, board_texture, NULL, NULL);  
      SDL_RenderTexture(renderer, highlight_texture, NULL, NULL);
      SDL_RenderTexture(renderer, highlight_piece_texture, NULL, NULL);
      SDL_RenderTexture(renderer, pieces_cache.texture, NULL, NULL); 
      SDL_RenderPresent(renderer);
      need_redraw = false;
    }

    frame_end = SDL_GetTicks();
    frame_time = frame_end - frame_start;

    if (frame_time < FRAME_TIME_MS) {
      SDL_Delay(FRAME_TIME_MS - frame_time); // Cap FPS
    } 
    fps_update_terminal(&fps_counter, FPS_UPDATE_INTERVAL);
  }

  destroy_cached_pieces(&pieces_cache);
  destroy_pieces_textures(&textures);
  SDL_DestroyTexture(board_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}


