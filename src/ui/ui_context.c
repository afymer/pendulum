#include "ui_context.h"

void ui_frame_begin(ui_context_t *cx)
{
    cx->frame_scratch = arena_temp_begin(cx->frame_arena);
}

void ui_frame_end(ui_context_t *cx)
{
    arena_temp_end(cx->frame_scratch);
}
