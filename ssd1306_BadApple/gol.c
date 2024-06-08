#include "gol.h"
#include <stdlib.h>

GOL_t* gol_init(size_t width, size_t height) {
    GOL_t* gol = malloc(sizeof(GOL_t));

    size_t size_of_buffer = sizeof(uint8_t) * width * height; 

    gol->map = malloc(size_of_buffer);
    gol->next = malloc(size_of_buffer);
    
    for (size_t i = 0; i < size_of_buffer; i++) {
        gol->map[i] = 0;
        gol->next[i] = 0;
    }


    gol->width = width;
    gol->height = height;
    gol->generation = 0;

    return gol;
}

static uint8_t gol_check_neighbors(GOL_t* state, int32_t x, int32_t y) {
    uint8_t count = 0;

    for (int8_t yO = -1; yO < 2; yO++) {
        for (int8_t xO = -1; xO < 2; xO++) {
            if (yO == 0 && xO == 0)
                continue;

            int32_t cx = (x + xO) % state->width;
            int32_t cy = (y + yO) % state->height;

            if (0 != state->map[cx + cy * state->width])
                count++;
        }
    }

    return count;
}

void gol_update(GOL_t* state) {
    size_t array_size = state->width * state->height;

    for (size_t i = 0; i < array_size; i++) {
        size_t x = i % state->width;
        size_t y = i / state->width;

        uint8_t neighbors = gol_check_neighbors(state, x, y);

        uint8_t cell_state = state->map[x + y * state->width];
        
        if (cell_state == 0 && neighbors == 3) {
            state->next[x + y * state->width] = 1;
        }
        else if (cell_state == 0) {
            state->next[x + y * state->width] = 0;
        }
        else if (neighbors >= 2 && neighbors <= 3) {
            state->next[x + y * state->width] = 1;
        }
        else {
            state->next[x + y * state->width] = 0;
        }
    }

    for (size_t i = 0; i < array_size; i++) {
        state->map[i] = state->next[i];
        state->next[i]= 0;
    }
}

