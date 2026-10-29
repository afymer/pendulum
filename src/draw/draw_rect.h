#pragma once

#include "draw_core.h"

void fill_rect(framebuffer_t fb, i32 x, i32 y, i32 w, i32 h, u32 color);
void blend_pixel(framebuffer_t fb, i32 x, i32 y, u32 color, f32 coverage);
