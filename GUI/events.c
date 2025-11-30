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

struct MoveList* select_pieces_moves(struct MoveList* moves_list, int board_index) {
  struct MoveList* piece_moves = malloc(sizeof(struct MoveList));
  piece_moves->count = 0;

  for (int i = 0; i < moves_list->count; i++) {
    uint32_t move = moves_list->moves[i];
    int from_square = move & 0x3F; // Bits 0-5
    if (from_square == board_index) {
      piece_moves->moves[piece_moves->count++] = move;
    }
  }
  return piece_moves;
}

uint32_t select_move_from_list(struct MoveList* moves_list, int board_index) {
  for (int i = 0; i < moves_list->count; i++) {
    uint32_t move = moves_list->moves[i];
    int to_square = (move >> 6) & 0x3F; // Bits 6-11
    if (to_square == board_index) {
      return move;
    }
  }
  return 0; // Return 0 if no move found
}


void main_switch_event(SDL_Event* event, bool* running, uint32_t *move,
                      struct MoveList* move_list, struct MoveList* move_list_piece, 
                      struct BoardStateUI* board_state_ui, struct RenderContext* render_context, 
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

      if(board_state_ui->selected_movable_piece == true) {
        uint32_t selected_move = select_move_from_list(move_list_piece, board_state_ui->board_index);

        if(selected_move) { 
          board_state_ui->piece_moved = true;
          *move = selected_move;
        } else {
          printf("No valid move to that square.\n");  
        }
        render_context->highlight_texture = create_highlight_texture(render_context->renderer, config->window_width, config->window_height, -1);
        board_state_ui->selected_movable_piece = false;
        render_context->highlight_piece_texture = NULL;
        board_state_ui->need_redraw = true;
        break;
      }

      // Select piece if it belongs to the player
      if(side_data->player_pieces & (1ULL << board_state_ui->board_index)) {
        // Highlight selected piece 
        *move_list_piece = *select_pieces_moves(move_list, board_state_ui->board_index);
        render_context->highlight_texture = create_highlight_texture(render_context->renderer, config->window_width, config->window_height, board_state_ui->board_index);
        render_context->highlight_piece_texture = create_piece_highlight_texture(render_context->renderer, config->window_width, config->window_height, move_list_piece);
        //print_move_list(move_list_piece);
        board_state_ui->selected_movable_piece = true;
      } else {
        render_context->highlight_texture = create_highlight_texture(render_context->renderer, config->window_width, config->window_height, -1);
        render_context->highlight_piece_texture = NULL;
      }
      board_state_ui->need_redraw = true;
      break;

    default:
      break;
  }
}
