#include "draw/draw_core.h"

void fill_rect(framebuffer_t fb, i32 x, i32 y, i32 w, i32 h, u32 color)
{
    i32 x0 = MAX(x, 0);
    i32 y0 = MAX(y, 0);
    i32 x1 = MIN(x + w, fb.width);
    i32 y1 = MIN(y + h, fb.height);

    for (i32 row = y0; row < y1; row++) {
        u32 *line = (u32 *)((u8 *)fb.pixels + (usize)row * (usize)fb.stride);
        for (i32 col = x0; col < x1; col++) {
            line[col] = color;
        }
    }
}
