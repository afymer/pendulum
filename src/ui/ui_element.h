#pragma once

#include "layout/layout_core.h"
#include "math/math_vec.h"
#include "ui/ui_slotmap.h"

typedef struct ui_cx_s ui_cx_t;

typedef enum {
    UI_ELEMENT_CONTAINER,
    UI_ELEMENT_TEXT,
    UI_ELEMENT_CUSTOM,
} ui_element_kind_t;

typedef struct ui_element_s ui_element_t;
struct ui_element_s {
    ui_element_kind_t kind;
    layout_style_t style;
    vec2f32 computed_pos;
    vec2f32 computed_size;

    ui_element_t *parent;
    ui_element_t *first_child;
    ui_element_t *last_child;
    ui_element_t *next_sibling;

    u32 background;
    void (*custom_draw)(ui_element_t *el, ui_cx_t *cx);
    void *user_data;
    ui_handle_t view_handle;
    bool is_dirty;
};
