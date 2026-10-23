#pragma once

#include "ui/ui_context.h"
#include "window/window_inc.h"
typedef struct window_cx_s window_cx_t;

struct window_cx_s {
    ui_context_t *app;
    window_t *window;
};
