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

void blend_pixel(framebuffer_t fb, i32 x, i32 y, u32 color, f32 coverage)
{
    if (x < 0 || y < 0 || x >= fb.width || y >= fb.height || coverage <= 0.0f) {
        return;
    }
    u32 *dst = (u32 *)((u8 *)fb.pixels + (usize)y * (usize)fb.stride) + x;
    u32 a = (u32)(MIN(coverage, 1.0f) * 256.0f + 0.5f);
    u32 d = *dst;
    u32 out = 0xFF000000;
    for (i32 shift = 0; shift < 24; shift += 8) {
        i32 s = (i32)((color >> shift) & 0xFF);
        i32 t = (i32)((d >> shift) & 0xFF);
        out |= (u32)(t + (((s - t) * (i32)a) >> 8)) << shift;
    }
    *dst = out;
}
