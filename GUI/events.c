// Event handling for the chess GUI

#include "gui_board.h"

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
struct MoveList selected_piece_moves(struct MoveList* moves_list, int board_index) {
    struct MoveList filtered_moves;
    filtered_moves.count = 0;

    for (int i = 0; i < moves_list->count; i++) {
        uint32_t move = moves_list->moves[i];
        int from_square = move & 0x3F; // Extract bits 0-5 for from square

        if (from_square == board_index) {
            filtered_moves.moves[filtered_moves.count++] = move;
        }
    }
    return filtered_moves;
}

