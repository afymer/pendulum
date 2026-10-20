#pragma once

#include "draw_core.h"

void draw_line(framebuffer_t fb, i32 x0, i32 y0, i32 x1, i32 y1, i32 thickness, u32 color);
void fill_circle(framebuffer_t fb, i32 cx, i32 cy, i32 radius, u32 color);
