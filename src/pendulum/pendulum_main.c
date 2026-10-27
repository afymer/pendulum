// headers
#include "base/base_inc.h"
#include "math/math_inc.h"
#include "draw/draw_inc.h"
#include "layout/layout_inc.h"
#include "sim/sim_inc.h"
#include "controller/controller_inc.h"
#include "window/window_inc.h"
#include "ui/ui_inc.h"

// units
#include "base/base_inc.c"
#include "math/math_inc.c"
#include "draw/draw_inc.c"
#include "layout/layout_inc.c"
#include "sim/sim_inc.c"
#include "controller/controller_inc.c"
#include "window/window_inc.c"
#include "ui/ui_inc.c"

#define SIM_DT_NS (1 * NS_PER_MS)
#define SIM_DT ((f32)SIM_DT_NS / (f32)NS_PER_SEC)
#define CONTROL_DIVIDER 5
#define CONTROL_DT (SIM_DT * CONTROL_DIVIDER)
#define TARGET_SWITCH_S 6.0f
#define MAX_FRAME_NS (250 * NS_PER_MS)

typedef struct counter_state_s {
    i32 count;
} counter_state_t;

static void counter_render(void *self, ui_cx_t *cx)
{
    (void)self;
    ui_push_container(cx,
                      (layout_style_t){
                          .axis = LAYOUT_AXIS_HORIZONTAL,
                          .gap = 8.0f,
                          .padding = 8.0f,
                      },
                      0xFF18181B);

    ui_append_child(cx, (layout_style_t){ .width = 220.0f }, 0x212121);
    ui_append_child(cx, (layout_style_t){ .grow = 1.0f }, 0x424242);
    ui_append_child(cx, (layout_style_t){ .width = 260.0f }, 0x212121);

    ui_pop(cx);
}

static const ui_view_vtable_t counter_view_vtable = { .render = counter_render, .destroy = NULL };

int main(void)
{
    arena_t *arena = arena_create((arena_params_t){
        .reserve_size = ARENA_DEFAULT_RESERVE_SIZE,
        .commit_size = ARENA_DEFAULT_COMMIT_SIZE,
    });
    matf32 m1 = id_matf32(arena, 2);
    matf32 m2 = create_matf32(arena, 2, 2);
    scale_matf32(m1, 2.0f);
    MAT_SET(m2, 0, 0, 1.0f);
    MAT_SET(m2, 0, 1, 2.0f);
    MAT_SET(m2, 1, 0, 3.0f);
    MAT_SET(m2, 1, 1, 4.0f);
    matf32 res = mul_matf32(arena, m1, m2);
    print_matf32(res);

    u64 last_ns = time_now_ns();
    u64 acc_ns = 0;
    pendulum_t pendulum = {
        .params = {
            .length = 0.6f,
            .bob_mass = 0.1f,
            .cart_mass = 0.5f,
            .gravity = v2f32(0.0f, -9.81f),
            .cart_friction = 0.1f,
            .max_force = 100.0f,
        },
        .state = { .theta = 3.1f },
    };

    const lqr_weights_t weights = { .q = { 10.0f, 1.0f, 100.0f, 1.0f }, .r = 0.1f };
    f32 gains[4];
    arena_t *scratch_arena = arena_create((arena_params_t){
        .reserve_size = 64 * 1024,
        .commit_size = 64 * 1024,
    });
    u32 design_iterations = lqr_solve(scratch_arena, &pendulum.params, weights, CONTROL_DT, gains);
    arena_free(scratch_arena);
    if (0 == design_iterations) {
        LOG_ERROR("LQR did not converge");
        return EXIT_FAILURE;
    }
    controller_feedback_t feedback;
    controller_feedback_init(&feedback, gains);
    controller_t *controller = &feedback.base;

    u64 sim_ticks = 0;
    f32 sim_time = 0.0f;
    f32 target_x = 0.0f;
    f32 force = 0.0f;

    ui_app_t *app = ui_app_create();

    counter_state_t counter_state = { .count = 0 };
    ui_view_t counter_view = { .entity = &counter_state, .vtable = &counter_view_vtable };
    ui_handle_t counter_handle = ui_slotmap_insert(app->views, &counter_view);

    window_t *window = window_create(
        app, (window_params_t){ .width = 800, .height = 600, .title = "Counter App" });
    window->root_view = counter_handle;

    ui_notify(app, counter_handle);

    while (window_is_open(window)) {
        ui_event_t event = wait_for_event();
        dispatch_event(app, window, &event);
        while (poll_event(&event)) {
            dispatch_event(app, window, &event);
        }

        u64 now_ns = time_now_ns();
        acc_ns += MIN(now_ns - last_ns, MAX_FRAME_NS);
        last_ns = now_ns;
        while (acc_ns >= SIM_DT_NS) {
            target_x = fmodf(sim_time, 2.0f * TARGET_SWITCH_S) < TARGET_SWITCH_S ? 1.0f : -1.0f;
            if (sim_ticks % CONTROL_DIVIDER == 0) {
                force = controller_update(controller, pendulum_measure(&pendulum), target_x,
                                          CONTROL_DT);
            }
            pendulum_step(&pendulum, force, SIM_DT);
            sim_ticks++;
            sim_time += SIM_DT;
            acc_ns -= SIM_DT_NS;
        }

        ui_window_render(app, window);
    }

    ui_app_destroy(app);
    return 0;
}
