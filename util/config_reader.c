// Reader for settings file

#include "util.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

int config_reader(const char* filename, struct Config* config) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[50];
        int value;
        if (sscanf(line, "%49[^=]=%d", key, &value) == 2) {
            if (strcmp(key, "window_width") == 0) {
                config->window_width = value;
                printf("Width set to %d\n", value);
            } else if (strcmp(key, "window_height") == 0) {
                config->window_height = value;
                printf("Height set to %d\n", value);
            } else if (strcmp(key, "fullscreen") == 0) {
                config->fullscreen = value;
                printf("Fullscreen set to %d\n", value);
            }
        }
    }

    fclose(file);
    return 0;
}



