#pragma once

typedef struct ui_context_s ui_context_t;
struct ui_context_s {
    arena_t *frame_arena;
    arena_temp_t frame_scratch;

    void *globals[32];
    struct ui_slotmap_s *entities;
};

void ui_frame_begin(ui_context_t *cx);
void ui_frame_end(ui_context_t *cx);
