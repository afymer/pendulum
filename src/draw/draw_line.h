#pragma once

#include "draw_core.h"

void draw_line(framebuffer_t fb, f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, u32 color);
void fill_circle(framebuffer_t fb, f32 cx, f32 cy, f32 radius, u32 color);
