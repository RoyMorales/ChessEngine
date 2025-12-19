#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include "../core/board.h"
#include "../core/move_gen.h"
#include "../core/move_filter.h"

#define MAX_MOVES 256


struct Config {
    int window_width;
    int window_height;
    int fullscreen;
};

struct FPSCounter {
    uint64_t last_time;
    uint64_t frame_count;
    double fps;
};

int config_reader(const char* filename, struct Config* config);

void fps_init(struct FPSCounter* c);
void fps_update_terminal(struct FPSCounter* c, double x);

#endif

