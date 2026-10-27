#pragma once

#include "draw/draw_core.h"
#include "layout/layout_core.h"
#include "math/math_vec.h"
#include "ui/ui_slotmap.h"

typedef struct ui_cx_s ui_cx_t;

typedef enum {
    UI_NODE_DIV,
    UI_NODE_CANVAS,
    UI_NODE_VIEW_MOUNT,
} ui_node_kind_t;

typedef void (*ui_canvas_draw_fn)(framebuffer_t fb, void *user_data);

typedef struct ui_element_s ui_element_t;
struct ui_element_s {
    ui_node_kind_t kind;
    layout_style_t style;
    vec2f32 computed_pos;
    vec2f32 computed_size;

    ui_element_t *parent;
    ui_element_t *next_sibling;
    ui_handle_t view_handle;
    bool is_dirty;

    union {
        struct {
            ui_element_t *first_child;
            ui_element_t *last_child;
            u32 background;
        } div;

        struct {
            ui_canvas_draw_fn draw;
            void *user_data;
            u32 background;
        } canvas;

        struct {
            ui_handle_t child_view;
        } view_mount;
    };
};
