#ifndef CAIRO_RENDERER_H
#define CAIRO_RENDERER_H

#include <stdint.h>

void drawCairoSquare(
    uint8_t *pool_data,
    int width,
    int height,
    int offset,
    int stride
);

#endif
