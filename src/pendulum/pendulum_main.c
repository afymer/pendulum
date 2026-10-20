// headers
#include "base/base_inc.h"
#include "math/math_inc.h"
#include "draw/draw_inc.h"
#include "sim/sim_inc.h"
#include "controller/controller_inc.h"
#include "window/window_inc.h"

// units
#include "base/base_inc.c"
#include "math/math_inc.c"
#include "draw/draw_inc.c"
#include "sim/sim_inc.c"
#include "controller/controller_inc.c"
#include "window/window_inc.c"

#define SIM_DT_NS (1 * NS_PER_MS)
#define SIM_DT ((f32)SIM_DT_NS / (f32)NS_PER_SEC)
#define CONTROL_DIVIDER 5
#define CONTROL_DT (SIM_DT * CONTROL_DIVIDER)
#define TARGET_SWITCH_S 6.0f
#define MAX_FRAME_NS (250 * NS_PER_MS)

int main()
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

    window_t *window = window_create(arena, (window_params_t){
                                                .title = "pendulum",
                                                .width = 1920,
                                                .height = 1080,
                                            });
    if (NULL == window) {
        return EXIT_FAILURE;
    }

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

    const f32 pixels_per_meter = 400.0f;
    matf32 world_to_screen = diag_matf32(arena, 2, pixels_per_meter);
    MAT_GET(world_to_screen, 1, 1) *= -1.f;

    while (!window->should_close) {
        if (window->needs_redraw) {
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

            framebuffer_t fb = window_framebuffer(window);
            fill_rect(fb, 0, 0, fb.width, fb.height, RGB(0x21, 0x21, 0x21));
            vec2f32 origin = scale_v2f32(v2f32((f32)fb.width, (f32)fb.height), 0.5f);
            vec2f32 root = add_v2f32(
                origin, mul_matf32_v2f32(world_to_screen, v2f32(pendulum.state.x, 0.0f)));
            vec2f32 bob =
                add_v2f32(origin, mul_matf32_v2f32(world_to_screen, pendulum_position(&pendulum)));
            vec2f32 target =
                add_v2f32(origin, mul_matf32_v2f32(world_to_screen, v2f32(target_x, 0.0f)));
            draw_line(fb, 0.0f, origin.y, (f32)fb.width, origin.y, 2.0f, RGB(0x55, 0x55, 0x55));
            fill_rect(fb, (i32)target.x - 2, (i32)origin.y - 20, 4, 40, RGB(0x66, 0xAA, 0x66));
            fill_rect(fb, (i32)root.x - 50, (i32)root.y - 15, 100, 30, RGB(0x99, 0x99, 0x99));
            draw_line(fb, root.x, root.y, bob.x, bob.y, 3.0f, RGB(0xCC, 0xCC, 0xCC));
            fill_circle(fb, bob.x, bob.y, 8.0f, RGB(0x88, 0x88, 0xFF));
            window_present(window);
        }
        window_wait_events(window);
    }
    window_destroy(window);
    return EXIT_SUCCESS;
}
