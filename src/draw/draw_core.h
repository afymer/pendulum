#pragma once

typedef struct framebuffer_s framebuffer_t;

struct framebuffer_s {
    u32 *pixels;
    i32 width;
    i32 height;
    i32 stride; // may be larger than width * 4
};
