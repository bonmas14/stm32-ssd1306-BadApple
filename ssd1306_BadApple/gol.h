#ifndef GOL_H_
#define GOL_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t generation;
    size_t width;
    size_t height;
    uint8_t* map;
    uint8_t* next;
} GOL_t;

GOL_t* gol_init(size_t width, size_t height);
void gol_update(GOL_t* state);

#endif
