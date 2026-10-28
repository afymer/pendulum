// headers
#include "base/base_inc.h"
#include "window/window_inc.h"
#include "ui/ui_inc.h"

#include "pendulum_sim.h"
#include "pendulum_canvas_view.h"
#include "pendulum_app_view.h"

// units
#include "base/base_inc.c"
#include "math/math_inc.c"
#include "draw/draw_inc.c"             // IWYU pragma: keep
#include "layout/layout_inc.c"         // IWYU pragma: keep
#include "sim/sim_inc.c"               // IWYU pragma: keep
#include "controller/controller_inc.c" // IWYU pragma: keep
#include "window/window_inc.c"
#include "ui/ui_inc.c"

#include "pendulum_sim.c"
#include "pendulum_canvas_view.c"
#include "pendulum_app_view.c"

int main(void)
{
    pendulum_sim_t sim;
    if (!pendulum_sim_init(&sim)) {
        return EXIT_FAILURE;
    }

    ui_app_t *app = ui_app_create();
    ui_handle_t canvas_view = pendulum_canvas_view_create(app, &sim, 350.0f);
    ui_handle_t app_view = pendulum_app_view_create(app, canvas_view);

    window_t *window = window_create(
        app, (window_params_t){ .width = 1280, .height = 720, .title = "Pendulum Simulation" });
    window->root_view = app_view;

    window_set_animating(window, true);
    ui_notify(app, app_view);

    u64 last_ns = time_now_ns();
    u64 acc_ns = 0;

    while (window_is_open(window)) {
        ui_event_t event = wait_for_event();
        dispatch_event(app, window, &event);
        while (poll_event(&event)) {
            dispatch_event(app, window, &event);
        }

        if (window->is_animating) {
            u64 now_ns = time_now_ns();
            acc_ns += MIN(now_ns - last_ns, MAX_FRAME_NS);
            last_ns = now_ns;
            while (acc_ns >= SIM_DT_NS) {
                pendulum_sim_tick(&sim);
                acc_ns -= SIM_DT_NS;
            }
            ui_notify(app, canvas_view);
        } else {
            last_ns = time_now_ns();
            acc_ns = 0;
        }

        ui_window_render(app, window);
    }

    ui_app_destroy(app);
    return EXIT_SUCCESS;
}
