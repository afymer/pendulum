#pragma once

#include "ui/ui_inc.h"

typedef struct pendulum_app_view_s {
    ui_handle_t canvas_view;
} pendulum_app_view_t;

ui_handle_t pendulum_app_view_create(ui_app_t *app, ui_handle_t canvas_view);
