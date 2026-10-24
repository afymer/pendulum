#pragma once

#include "ui/ui_element.h"
#include "ui/ui_slotmap.h"
#include "window/window_core.h"

#define UI_MAX_WINDOWS 8
#define UI_MAX_VIEWS 256
#define UI_MAX_ENTITIES 256
#define UI_ENTITY_SIZE 64
#define UI_ARENA_SIZE (1024 * 1024)
#define UI_MAX_PARENT_DEPTH 64

typedef struct ui_app_s ui_app_t;
struct ui_app_s {
    arena_t *arena;
    arena_t *frame_arena;

    ui_slotmap_t *entities;
    ui_slotmap_t *views;
    void *globals[32];

    window_t *windows[UI_MAX_WINDOWS];
    u32 window_count;
};

struct ui_cx_s {
    ui_app_t *app;
    window_t *window;
    ui_element_t *root;
    ui_element_t *parent_stack[UI_MAX_PARENT_DEPTH];
    u32 parent_depth;
};

typedef void (*ui_view_render_fn)(void *self, ui_cx_t *cx);

typedef struct ui_view_vtable_s ui_view_vtable_t;
struct ui_view_vtable_s {
    ui_view_render_fn render;
    void (*destroy)(void *self);
};

typedef struct ui_view_s ui_view_t;
struct ui_view_s {
    void *entity;
    const ui_view_vtable_t *vtable;
};

ui_app_t *ui_app_create(void);
void ui_app_destroy(ui_app_t *app);
void ui_app_add_window(ui_app_t *app, window_t *window);

void ui_notify(ui_app_t *app, ui_handle_t handle);

ui_element_t *ui_push_container(ui_cx_t *cx, layout_style_t style, u32 background);
void ui_pop(ui_cx_t *cx);
ui_element_t *ui_append_child(ui_cx_t *cx, layout_style_t style, u32 background);

void ui_layout_solve(ui_element_t *root, f32 width, f32 height);
void ui_element_paint(ui_cx_t *cx, ui_element_t *root);
void ui_window_render(ui_app_t *app, window_t *win);
