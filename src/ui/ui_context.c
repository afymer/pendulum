#include "ui_context.h"
#include "window/window_inc.h"

ui_context_t *ui_context_create(void)
{
    arena_t *arena = arena_create((arena_params_t){
        .reserve_size = UI_ARENA_SIZE,
        .commit_size = UI_ARENA_SIZE,
    });
    if (NULL == arena) {
        return NULL;
    }
    ui_context_t *cx = PUSH_STRUCT(arena, ui_context_t);
    *cx = (ui_context_t){ .arena = arena };

    cx->frame_arena = arena_create((arena_params_t){
        .reserve_size = UI_ARENA_SIZE,
        .commit_size = UI_ARENA_SIZE,
    });
    cx->entities = ui_slotmap_create(arena, UI_MAX_ENTITIES, UI_ENTITY_SIZE);
    cx->views = ui_slotmap_create(arena, UI_MAX_VIEWS, sizeof(ui_view_t));
    return cx;
}

void ui_context_destroy(ui_context_t *cx)
{
    if (NULL == cx) {
        return;
    }
    arena_free(cx->frame_arena);
    arena_free(cx->arena);
}

void ui_context_add_window(ui_context_t *cx, window_t *window)
{
    if (cx->window_count < UI_MAX_WINDOWS) {
        cx->windows[cx->window_count++] = window;
    }
}

void ui_notify(ui_context_t *cx, ui_handle_t handle)
{
    (void)handle;
    for (u32 i = 0; i < cx->window_count; i++) {
        cx->windows[i]->is_dirty = true;
    }
}

void ui_frame_begin(ui_context_t *cx)
{
    cx->frame_scratch = arena_temp_begin(cx->frame_arena);
}

void ui_frame_end(ui_context_t *cx)
{
    arena_temp_end(cx->frame_scratch);
}
