#pragma once

#include "ui/ui_slotmap.h"

typedef struct ui_context_s ui_context_t;
struct ui_context_s {
    arena_t *frame_arena;
    arena_temp_t frame_scratch;

    void *globals[32];
    ui_slotmap_t *entities;
};

void ui_frame_begin(ui_context_t *cx);
void ui_frame_end(ui_context_t *cx);
