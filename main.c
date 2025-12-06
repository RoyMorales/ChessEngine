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
#include "core/move_apply.h"

#define FPS_UPDATE_INTERVAL 10
#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)


int main(void) {
  struct Config config; 
  struct FPSCounter fps_counter;
  struct BoardStateUI board_state_ui;
  struct RenderContext render_context;
  struct SideData side_data;

  struct MoveList move_list;
  struct MoveList move_list_piece;

  printf("\n----------------------------\n");
  printf("         Chess Engine      \n");
  printf("----------------------------\n");
  
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

  render_context.renderer = SDL_CreateRendererWithProperties(renderer_props);
  printf("Renderer backend: %s\n", SDL_GetRendererName(render_context.renderer));

  SDL_DestroyProperties(renderer_props);

  if (!render_context.renderer) {
    fprintf(stderr, "SDL_CreateRendererWithProperties failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 4;
  }

  SDL_PropertiesID rprops = SDL_GetRendererProperties(render_context.renderer);
  bool vsync_enabled = SDL_GetNumberProperty(rprops, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 0);
  printf("VSync enabled: %s\n", vsync_enabled ? "true" : "false");
  
  printf("----------------------------\n");

  // Initialize game board from FEN
  //    Normal Setup
  char fen_setup[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  
  //char fen_setup[] = "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 w KQkq - 0 1";
  //char fen_setup[] = "8/8/8/3N4/8/8/8/7N w KQkq - 0 1";
  
  //    To Test Castle rights
  //char fen_setup[] = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
  
  struct Board game_board = fen_to_bitboards(fen_setup); 
  update_occupancy(&game_board);
  init_attack_tables();

  // Load GUI assets
  SDL_Event event; 
  struct ChessTextures textures = load_pieces_textures(render_context.renderer);
  render_context.board_texture = create_chessboard_texture(render_context.renderer, config.window_width, config.window_height);
  render_context.highlight_texture = create_highlight_texture(render_context.renderer, config.window_width, config.window_height, -1);
  render_context.highlight_piece_texture = NULL;
  render_context.pieces_cache = init_cached_pieces(render_context.renderer, &textures, &game_board, config.window_width, config.window_height);

  // FPS management variables
  uint64_t frame_start, frame_end, frame_time;
  fps_init(&fps_counter);

  // Main loop variables
  board_state_ui.need_redraw = true;
  board_state_ui.selected_movable_piece = false;
  board_state_ui.piece_moved = false;

  // ToDO! Implement player choice
  char player = white_player;
  char opponent = (player == white_player) ? black_player : white_player;
  uint64_t player_pieces = (player == white_player) ? game_board.white_occupied : game_board.black_occupied; 
  uint64_t opponent_pieces = (opponent == white_player) ? game_board.white_occupied : game_board.black_occupied;
  side_data.player_pieces = player_pieces;
  side_data.opponent_pieces = opponent_pieces;

  bool running = true;
  bool generate_moves = true;
  uint32_t move = 0;

  while (running) {
    frame_start = SDL_GetTicks();

    // Generate moves if needed
    if (generate_moves) {
      move_list = genrate_board_moves(&game_board);
      printf("----------------------------\n");
      printf("Generated %d moves for player %s\n", move_list.count, (game_board.player_turn == white_player) ? "White" : "Black");
      printf("----------------------------\n");
      print_move_list(&move_list);
      generate_moves = false;
    }

    // Event handling
    while (SDL_PollEvent(&event)) {
      main_switch_event(&event, &running, &move, &move_list, &move_list_piece,
                        &board_state_ui, &render_context, &config, &side_data);
      }

    // Redraw if needed
    if (board_state_ui.need_redraw) {
      SDL_RenderTexture(render_context.renderer, render_context.board_texture, NULL, NULL);  
      SDL_RenderTexture(render_context.renderer, render_context.highlight_texture, NULL, NULL);
      SDL_RenderTexture(render_context.renderer, render_context.highlight_piece_texture, NULL, NULL);
      SDL_RenderTexture(render_context.renderer, render_context.pieces_cache.texture, NULL, NULL); 
      SDL_RenderPresent(render_context.renderer);
      board_state_ui.need_redraw = false;
    }

    // Apply move if piece was moved
    if(board_state_ui.piece_moved) {
      printf("Selected Move: From %d to %d\n\n", move & 0x3F, (move >> 6) & 0x3F);
      apply_move(&game_board, move);
      update_occupancy(&game_board);

      destroy_cached_pieces(&render_context.pieces_cache);
      render_context.pieces_cache = init_cached_pieces(render_context.renderer, &textures, &game_board, 
                                                      config.window_height, config.window_width);

      side_data.player_pieces = (game_board.player_turn == white_player) ? game_board.white_occupied : game_board.black_occupied; 
      side_data.opponent_pieces = (game_board.player_turn == white_player) ? game_board.black_occupied : game_board.white_occupied;
      
      board_state_ui.piece_moved = false;
      board_state_ui.need_redraw = true;
      generate_moves = true;
    }

    frame_end = SDL_GetTicks();
    frame_time = frame_end - frame_start;
    if (frame_time < FRAME_TIME_MS) {
      SDL_Delay(FRAME_TIME_MS - frame_time); // Cap FPS
    } 
    //fps_update_terminal(&fps_counter, FPS_UPDATE_INTERVAL);
  }

  destroy_pieces_textures(&textures);
  destroy_cached_pieces(&render_context.pieces_cache); 
  SDL_DestroyTexture(render_context.board_texture);
  SDL_DestroyRenderer(render_context.renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}


