#include "draw/draw_rect.h"
#include "draw/draw_line.h"

void draw_line(framebuffer_t fb, i32 x0, i32 y0, i32 x1, i32 y1, i32 thickness, u32 color)
{
    i32 dx = ABS(x1 - x0);
    i32 dy = -ABS(y1 - y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx + dy;
    i32 half = thickness / 2;

    for (;;) {
        fill_rect(fb, x0 - half, y0 - half, thickness, thickness, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        i32 e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void fill_circle(framebuffer_t fb, i32 cx, i32 cy, i32 radius, u32 color)
{
    for (i32 dy = -radius; dy <= radius; dy++) {
        i32 dx = (i32)sqrtf((f32)(radius * radius - dy * dy));
        fill_rect(fb, cx - dx, cy + dy, 2 * dx + 1, 1, color);
    }
}
