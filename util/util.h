#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

typedef struct {
    int window_width;
    int window_height;
    int fullscreen;
} Config;

typedef struct {
    uint64_t last_time;
    uint64_t frame_count;
    double fps;
} FPSCounter;

int config_reader(const char* filename, Config* config);
void fps_init(FPSCounter* c);
void fps_update_terminal(FPSCounter* c, double x);

#endif

