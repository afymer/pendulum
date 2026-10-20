#include "math_vec.h"

static inline vec2f32 v2f32(f32 a, f32 b)
{
    return (vec2f32){ a, b };
}

static inline vec2f32 add_v2f32(vec2f32 a, vec2f32 b)
{
    return (vec2f32){ a.x + b.x, a.y + b.y };
}
static inline vec2f32 sub_v2f32(vec2f32 a, vec2f32 b)
{
    return (vec2f32){ a.x - b.x, a.y - b.y };
}
static inline vec2f32 mul_v2f32(vec2f32 a, vec2f32 b)
{
    return (vec2f32){ a.x * b.x, a.y * b.y };
}
static inline void print_v2f32(vec2f32 a)
{
    printf("(%f, %f)\n", a.x, a.y);
}
static inline f32 dot_v2f32(vec2f32 a, vec2f32 b)
{
    return a.x * b.x + a.y * b.y;
}
