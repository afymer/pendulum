// headers
#include "base/base_inc.h"
#include "window/window_inc.h"
#include "ui/ui_inc.h"

#include "pendulum_sim.h"
#include "pendulum_canvas_view.h"

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

int main(void)
{
    pendulum_sim_t sim;
    if (!pendulum_sim_init(&sim)) {
        return EXIT_FAILURE;
    }

    ui_app_t *app = ui_app_create();
    ui_handle_t canvas_view = pendulum_canvas_view_create(app, &sim, 350.0f);

    window_t *window = window_create(
        app, (window_params_t){ .width = 1280, .height = 720, .title = "Pendulum Simulation" });

    window_set_animating(window, true);

    bool show_menu = true;

    u64 last_ns = time_now_ns();
    u64 acc_ns = 0;

    while (window_is_open(window)) {
        ui_event_t event = wait_for_event();
        if (event.kind == UI_EVENT_KEY && event.key.is_down) {
            show_menu = !show_menu;
            ui_window_invalidate(window);
        }
        dispatch_event(app, window, &event);
        while (poll_event(&event)) {
            if (event.kind == UI_EVENT_KEY && event.key.is_down) {
                show_menu = !show_menu;
                ui_window_invalidate(window);
            }
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

        ui_cx_t *cx = ui_begin_window(app, window);
        if (cx) {
            ui_div_begin(cx, (ui_div_params_t){
                                 .style = {
                                     .axis = LAYOUT_AXIS_VERTICAL,
                                     .grow = 1.0f,
                                 },
                                 .background = 0xFF18181B,
                             });
            {
                if (show_menu) {
                    ui_div_begin(cx, (ui_div_params_t){
                                         .style = {
                                             .height = 36.0f,
                                             .axis = LAYOUT_AXIS_HORIZONTAL,
                                             .gap = 8.0f,
                                             .padding = 6.0f,
                                         },
                                         .background = 0xFF27272A,
                                     });
                    {
                        ui_div(cx, (ui_div_params_t){ .style = { .width = 64.0f },
                                                      .background = 0xFF3F3F46 });
                        ui_div(cx, (ui_div_params_t){ .style = { .width = 64.0f },
                                                      .background = 0xFF3F3F46 });
                        ui_div(cx, (ui_div_params_t){ .style = { .width = 64.0f },
                                                      .background = 0xFF3F3F46 });
                    }
                    ui_div_end(cx);
                }

                ui_div_begin(cx, (ui_div_params_t){
                                     .style = {
                                         .axis = LAYOUT_AXIS_HORIZONTAL,
                                         .gap = 8.0f,
                                         .padding = 8.0f,
                                         .grow = 1.0f,
                                     },
                                     .background = 0xFF18181B,
                                 });
                {
                    ui_div(cx, (ui_div_params_t){ .style = { .width = 220.0f },
                                                  .background = 0xFF212121 });
                    ui_view(cx, canvas_view, (layout_style_t){ .grow = 1.0f });
                    ui_div(cx, (ui_div_params_t){ .style = { .width = 260.0f },
                                                  .background = 0xFF212121 });
                }
                ui_div_end(cx);
            }
            ui_div_end(cx);

            ui_end_window(cx);
        }
    }

    ui_app_destroy(app);
    return EXIT_SUCCESS;
}
