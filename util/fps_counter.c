#include <SDL3/SDL.h>
#include <stdio.h>

#include "util.h"

void fps_init(struct FPSCounter* c) {
    c->last_time = SDL_GetPerformanceCounter();
    c->frame_count = 0;
    c->fps = 0.0;
}

void fps_update_terminal(struct FPSCounter* c, double x) {
    c->frame_count++;

    uint64_t now = SDL_GetPerformanceCounter();
    double elapsed = (double)(now - c->last_time) /
                     (double)SDL_GetPerformanceFrequency();

    if (elapsed >= x) {                     // every x second
        c->fps = c->frame_count / elapsed;    // compute fps
        printf("FPS: %.1f\n", c->fps);        // print to console
        c->frame_count = 0;
        c->last_time = now;
    }
}


