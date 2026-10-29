#pragma once

#include "math/math_vec.h"

typedef enum layout_axis_e {
    LAYOUT_AXIS_VERTICAL,
    LAYOUT_AXIS_HORIZONTAL,
} layout_axis_t;

typedef struct layout_style_s layout_style_t;
struct layout_style_s {
    layout_axis_t axis;
    f32 width;
    f32 height;
    f32 padding;
    f32 gap;
    f32 grow;
};

typedef struct layout_node_s layout_node_t;
struct layout_node_s {
    layout_style_t style;

    layout_node_t *parent;
    layout_node_t *first_child;
    layout_node_t *last_child;
    layout_node_t *next_sibling;

    vec2f32 size;
    vec2f32 pos;
};

layout_node_t *layout_node_push(arena_t *arena, layout_node_t *parent, layout_style_t style);
void layout_solve(layout_node_t *root, f32 width, f32 height);
