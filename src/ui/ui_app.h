#pragma once

#include "draw/draw_core.h"
#include "ui/ui_element.h"
#include "ui/ui_slotmap.h"
#include "window/window_core.h"

#define UI_MAX_WINDOWS 8
#define UI_MAX_VIEWS 256
#define UI_MAX_ENTITIES 256
#define UI_ENTITY_SIZE 64
#define UI_ARENA_SIZE (1024 * 1024)
#define UI_VIEW_ARENA_SIZE (64 * 1024)
#define UI_VIEW_COMMIT_SIZE 4096
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
    arena_t *arena;
    framebuffer_t fb;
    ui_element_t *root;
    ui_element_t *parent_stack[UI_MAX_PARENT_DEPTH];
    u32 parent_depth;
    ui_handle_t current_view;
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
    arena_t *arena;
    ui_element_t *container;
    ui_element_t *root_element;
    u32 dirty_buffers;
    bool is_dirty;
};

typedef struct ui_div_params_s {
    layout_style_t style;
    u32 background;
} ui_div_params_t;

typedef struct ui_canvas_params_s {
    layout_style_t style;
    u32 background;
    ui_canvas_draw_fn draw;
    void *user_data;
} ui_canvas_params_t;

ui_app_t *ui_app_create(void);
void ui_app_destroy(ui_app_t *app);
void ui_app_add_window(ui_app_t *app, window_t *window);

ui_handle_t ui_view_create(ui_app_t *app, void *entity, const ui_view_vtable_t *vtable);
void ui_view_destroy(ui_app_t *app, ui_handle_t handle);
void ui_notify(ui_app_t *app, ui_handle_t handle);

ui_element_t *ui_div_begin(ui_cx_t *cx, ui_div_params_t params);
void ui_div_end(ui_cx_t *cx);
ui_element_t *ui_div(ui_cx_t *cx, ui_div_params_t params);
ui_element_t *ui_canvas(ui_cx_t *cx, ui_canvas_params_t params);
ui_element_t *ui_view(ui_cx_t *cx, ui_handle_t view_handle, layout_style_t style);

ui_element_t *ui_push_container(ui_cx_t *cx, layout_style_t style, u32 background);
void ui_pop(ui_cx_t *cx);
ui_element_t *ui_append_child(ui_cx_t *cx, layout_style_t style, u32 background);
ui_element_t *ui_append_view(ui_cx_t *cx, ui_handle_t view_handle, layout_style_t style);

void ui_layout_solve(ui_element_t *root, f32 width, f32 height);
void ui_element_paint(ui_cx_t *cx, ui_element_t *root);
void ui_window_render(ui_app_t *app, window_t *win);
