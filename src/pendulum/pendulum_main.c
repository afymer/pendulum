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

typedef struct sim_component_s {
    pendulum_t *pendulum;
    f32 target_x;
    f32 pixels_per_meter;
} sim_component_t;

static void sim_canvas_draw(framebuffer_t fb, void *user_data)
{
    sim_component_t *sim = user_data;
    if (NULL == sim || NULL == sim->pendulum) {
        return;
    }
    pendulum_t *pendulum = sim->pendulum;
    f32 ppm = sim->pixels_per_meter > 0.0f ? sim->pixels_per_meter : 350.0f;

    vec2f32 origin = v2f32((f32)fb.width * 0.5f, (f32)fb.height * 0.5f);

    vec2f32 root = v2f32(origin.x + pendulum->state.x * ppm, origin.y);
    vec2f32 rel = pendulum_position(pendulum);
    vec2f32 bob = v2f32(origin.x + rel.x * ppm, origin.y - rel.y * ppm);
    vec2f32 target = v2f32(origin.x + sim->target_x * ppm, origin.y);

    draw_line(fb, 0.0f, origin.y, (f32)fb.width, origin.y, 2.0f, 0xFF555555);
    fill_rect(fb, (i32)target.x - 2, (i32)origin.y - 20, 4, 40, 0xFF66AA66);
    fill_rect(fb, (i32)root.x - 50, (i32)root.y - 15, 100, 30, 0xFF999999);
    draw_line(fb, root.x, root.y, bob.x, bob.y, 3.0f, 0xFFCCCCCC);
    fill_circle(fb, bob.x, bob.y, 8.0f, 0xFF8888FF);
}

static void sim_render(void *self, ui_cx_t *cx)
{
    sim_component_t *sim = self;
    ui_canvas(cx, (ui_canvas_params_t){
                      .style = { .grow = 1.0f },
                      .background = 0xFF212121,
                      .draw = sim_canvas_draw,
                      .user_data = sim,
                  });
}

static const ui_view_vtable_t sim_view_vtable = {
    .render = sim_render,
    .destroy = NULL,
};

typedef struct app_state_s {
    ui_handle_t sim_view;
} app_state_t;

static void app_render(void *self, ui_cx_t *cx)
{
    app_state_t *state = self;
    ui_div_begin(cx, (ui_div_params_t){
                         .style = {
                             .axis = LAYOUT_AXIS_HORIZONTAL,
                             .gap = 8.0f,
                             .padding = 8.0f,
                         },
                         .background = 0xFF18181B,
                     });

    ui_div(cx, (ui_div_params_t){ .style = { .width = 220.0f }, .background = 0xFF212121 });
    ui_view(cx, state->sim_view, (layout_style_t){ .grow = 1.0f });
    ui_div(cx, (ui_div_params_t){ .style = { .width = 260.0f }, .background = 0xFF212121 });

    ui_div_end(cx);
}

static const ui_view_vtable_t app_view_vtable = {
    .render = app_render,
    .destroy = NULL,
};

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

    sim_component_t sim_comp = {
        .pendulum = &pendulum,
        .target_x = 0.0f,
        .pixels_per_meter = 350.0f,
    };
    ui_handle_t sim_handle = ui_view_create(app, &sim_comp, &sim_view_vtable);

    app_state_t app_state = {
        .sim_view = sim_handle,
    };
    ui_handle_t app_handle = ui_view_create(app, &app_state, &app_view_vtable);

    window_t *window = window_create(
        app, (window_params_t){ .width = 1280, .height = 720, .title = "Pendulum Simulation" });
    window->root_view = app_handle;

    window_set_animating(window, true);
    ui_notify(app, app_handle);

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
                target_x =
                    fmodf(sim_time, 2.0f * TARGET_SWITCH_S) < TARGET_SWITCH_S ? 1.0f : -1.0f;
                if (sim_ticks % CONTROL_DIVIDER == 0) {
                    force = controller_update(controller, pendulum_measure(&pendulum), target_x,
                                              CONTROL_DT);
                }
                pendulum_step(&pendulum, force, SIM_DT);
                sim_ticks++;
                sim_time += SIM_DT;
                acc_ns -= SIM_DT_NS;
            }
            sim_comp.target_x = target_x;
            ui_notify(app, sim_handle);
        } else {
            last_ns = time_now_ns();
            acc_ns = 0;
        }

        ui_window_render(app, window);
    }

    ui_app_destroy(app);
    arena_free(arena);
    return 0;
}
