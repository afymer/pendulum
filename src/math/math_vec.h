#pragma once

typedef struct vec2f32 vec2f32;
struct vec2f32 {
    f32 x;
    f32 y;
};

static vec2f32 v2f32(f32 a, f32 b);
static vec2f32 add_v2f32(vec2f32 a, vec2f32 b);
static vec2f32 sub_v2f32(vec2f32 a, vec2f32 b);
static vec2f32 mul_v2f32(vec2f32 a, vec2f32 b);
static void print_v2f32(vec2f32 a);
static f32 dot_v2f32(vec2f32 a, vec2f32 b);
