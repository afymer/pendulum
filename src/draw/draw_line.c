#include "draw/draw_rect.h"
#include "draw/draw_line.h"

static f32 coverage_from_distance(f32 d)
{
    f32 c = 0.5f - d;
    return c < 0.0f ? 0.0f : (c > 1.0f ? 1.0f : c);
}

void draw_line(framebuffer_t fb, f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, u32 color)
{
    f32 r = thickness * 0.5f;
    f32 vx = x1 - x0;
    f32 vy = y1 - y0;
    f32 len2 = vx * vx + vy * vy;
    f32 inv_len2 = len2 > 0.0f ? 1.0f / len2 : 0.0f;

    i32 px0 = MAX((i32)floorf(MIN(x0, x1) - r - 1.0f), 0);
    i32 py0 = MAX((i32)floorf(MIN(y0, y1) - r - 1.0f), 0);
    i32 px1 = MIN((i32)ceilf(MAX(x0, x1) + r + 1.0f), fb.width);
    i32 py1 = MIN((i32)ceilf(MAX(y0, y1) + r + 1.0f), fb.height);

    for (i32 y = py0; y < py1; y++) {
        for (i32 x = px0; x < px1; x++) {
            f32 wx = (f32)x + 0.5f - x0;
            f32 wy = (f32)y + 0.5f - y0;
            f32 t = (wx * vx + wy * vy) * inv_len2;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            f32 dx = wx - t * vx;
            f32 dy = wy - t * vy;
            f32 cov = coverage_from_distance(sqrtf(dx * dx + dy * dy) - r);
            blend_pixel(fb, x, y, color, cov);
        }
    }
}

void fill_circle(framebuffer_t fb, f32 cx, f32 cy, f32 radius, u32 color)
{
    f32 r_in = radius - 0.5f;
    f32 r_out = radius + 0.5f;
    i32 py0 = MAX((i32)floorf(cy - r_out), 0);
    i32 py1 = MIN((i32)ceilf(cy + r_out), fb.height);

    for (i32 y = py0; y < py1; y++) {
        f32 dy = (f32)y + 0.5f - cy;
        f32 out2 = r_out * r_out - dy * dy;
        if (out2 <= 0.0f) {
            continue;
        }
        f32 hw_out = sqrtf(out2);
        i32 x_lo = (i32)floorf(cx - hw_out);
        i32 x_hi = (i32)ceilf(cx + hw_out);

        i32 s_lo = x_hi;
        i32 s_hi = x_hi;
        f32 in2 = r_in * r_in - dy * dy;
        if (r_in > 0.0f && in2 > 0.0f) {
            f32 hw_in = sqrtf(in2);
            s_lo = MAX((i32)ceilf(cx - hw_in - 0.5f), x_lo);
            s_hi = MIN((i32)floorf(cx + hw_in - 0.5f) + 1, x_hi);
            if (s_hi < s_lo) {
                s_lo = s_hi = x_hi;
            }
        }

        for (i32 x = x_lo; x < s_lo; x++) {
            f32 dx = (f32)x + 0.5f - cx;
            blend_pixel(fb, x, y, color,
                        coverage_from_distance(sqrtf(dx * dx + dy * dy) - radius));
        }
        fill_rect(fb, s_lo, y, s_hi - s_lo, 1, color);
        for (i32 x = s_hi; x < x_hi; x++) {
            f32 dx = (f32)x + 0.5f - cx;
            blend_pixel(fb, x, y, color,
                        coverage_from_distance(sqrtf(dx * dx + dy * dy) - radius));
        }
    }
}
