// Event handling for the chess GUI

#include "gui_board.h"
#include "../core/core_util.h"
#include <stdio.h>
#include <stdlib.h>

void mouse_click_to_board_pos(float mouse_x, float mouse_y, int window_width, int window_height, int* board_x, int* board_y) {
  int tile_size = (window_width < window_height ? window_width : window_height) / 8;
  *board_x = ((int)(mouse_x) / tile_size);
  *board_y = 7 - ((int)(mouse_y) / tile_size);
  if (*board_x < 0) {
    *board_x = 0;
  } else if (*board_x > 7) {
    *board_x = 7;
  } 
  if (*board_y < 0) {
    *board_y = 0;
  } else if (*board_y > 7) {
    *board_y = 7; 
  }
}

// Creates MoveList from the original MoveList that do not originate from the selected square
struct MoveList* selected_piece_moves(struct MoveList* moves_list, int board_index) {
    struct MoveList* filtered_moves = malloc(sizeof(struct MoveList));
    filtered_moves->count = 0;

    for (int i = 0; i < moves_list->count; i++) {
        uint32_t move = moves_list->moves[i];
        int from_square = move & 0x3F; // Extract bits 0-5 for from square

        if (from_square == board_index) {
            filtered_moves->moves[filtered_moves->count++] = move;
        }
    }
    return filtered_moves;
}


void main_switch_event(SDL_Event* event, bool* running, bool* generate_moves, struct BoardStateUI* board_state_ui,
                      struct MoveData* move_data, struct RenderContext* render_context,
                      struct Config* config, struct SideData* side_data){

  switch (event->type) {
    case SDL_EVENT_QUIT:
      *running = false;
      break;

    case SDL_EVENT_KEY_DOWN:  
      if (event->key.key == SDLK_ESCAPE) {
        *running = false;
      }
      break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      mouse_click_to_board_pos(event->button.x, event->button.y,
                               config->window_width, config->window_height,
                               &board_state_ui->board_x, &board_state_ui->board_y);
      board_state_ui->board_index = board_state_ui->board_y  * 8 + board_state_ui->board_x;
      printf("Board Position: (%d, %d) ", board_state_ui->board_x, board_state_ui->board_y);
      printf("| Square Index: %d\n", board_state_ui->board_index);

      if(side_data->player_pieces & (1ULL << board_state_ui->board_index)) {
        // Highlight selected piece
        render_context->highlight_texture = create_highlight_texture(render_context->renderer, config->window_width, config->window_height, board_state_ui->board_index);
        
        // Hightlight possible moves
        move_data->piece_move_list = selected_piece_moves(&move_data->moves, board_state_ui->board_index);
        print_move_list(move_data->piece_move_list);
        render_context->highlight_piece_texture = create_piece_highlight_texture(render_context->renderer, 
                                                                config->window_width, config->window_height, 
                                                                move_data->piece_move_list);
        board_state_ui->need_redraw = true;  
        board_state_ui->selected_movable_piece = true;

      } else {
        render_context->highlight_texture = create_highlight_texture(render_context->renderer, config->window_width, config->window_height, -1);
        board_state_ui->need_redraw = true;
      }
      break;

    default:
      break;
  }
}
