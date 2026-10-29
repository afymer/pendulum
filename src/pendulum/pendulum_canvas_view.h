#pragma once

#include "base/base_inc.h"
#include "pendulum_sim.h"
#include "ui/ui_inc.h"

typedef struct pendulum_canvas_view_s {
    const pendulum_sim_t *sim;
    f32 pixels_per_meter;
} pendulum_canvas_view_t;

ui_handle_t pendulum_canvas_view_create(ui_app_t *app, const pendulum_sim_t *sim,
                                        f32 pixels_per_meter);
