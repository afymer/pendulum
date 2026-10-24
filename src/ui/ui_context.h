#pragma once

#include "ui/ui_slotmap.h"

#define UI_MAX_WINDOWS 8
#define UI_MAX_VIEWS 256
#define UI_MAX_ENTITIES 256
#define UI_ENTITY_SIZE 64
#define UI_ARENA_SIZE (1024 * 1024)

typedef struct window_s window_t;
typedef struct ui_context_s ui_context_t;

typedef struct ui_window_cx_s ui_window_cx_t;
struct ui_window_cx_s {
    ui_context_t *app;
    window_t *window;
};

typedef struct ui_view_vtable_s ui_view_vtable_t;
struct ui_view_vtable_s {
    void (*render)(void *state, ui_window_cx_t *wcx);
};

typedef struct ui_view_s ui_view_t;
struct ui_view_s {
    const ui_view_vtable_t *vtable;
    void *state;
};

struct ui_context_s {
    arena_t *arena;
    arena_t *frame_arena;
    arena_temp_t frame_scratch;

    void *globals[32];
    ui_slotmap_t *entities;
    ui_slotmap_t *views;

    window_t *windows[UI_MAX_WINDOWS];
    u32 window_count;
};

ui_context_t *ui_context_create(void);
void ui_context_destroy(ui_context_t *cx);
void ui_context_add_window(ui_context_t *cx, window_t *window);

void ui_notify(ui_context_t *cx, ui_handle_t handle);

void ui_frame_begin(ui_context_t *cx);
void ui_frame_end(ui_context_t *cx);
