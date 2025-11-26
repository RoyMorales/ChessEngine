// Event handling for the chess GUI

#include "gui_board.h"

void mouse_click_to_board_pos(float mouse_x, float mouse_y, int window_width, int window_height, int* board_x, int* board_y) {
  int tile_size = (window_width < window_height ? window_width : window_height) / 8;
  *board_x = ((int)(mouse_x) / tile_size) + 1;
  *board_y = 8 - ((int)(mouse_y) / tile_size);
  if (*board_x < 1) {
    *board_x = 1;
  } else if (*board_x > 8) {
    *board_x = 8;
  } 
  if (*board_y < 1) {
    *board_y = 1;
  } else if (*board_y > 8) {
    *board_y = 8; 
  }
}
