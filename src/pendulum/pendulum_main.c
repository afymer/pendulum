// headers
#include "base/base_inc.h"
#include "math/math_inc.h"
#include "draw/draw_inc.h"
#include "sim/sim_inc.h"
#include "window/window_inc.h"

// units
#include "base/base_inc.c"
#include "math/math_inc.c"
#include "draw/draw_inc.c"
#include "sim/sim_inc.c"
#include "window/window_inc.c"

#define SIM_DT_NS (1 * NS_PER_MS)
#define SIM_DT ((f32)SIM_DT_NS / (f32)NS_PER_SEC)
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
        .params = { .length = 1.0f, .gravity = v2f32(0.0f, -9.81f) },
        .state = { .theta = 3.1f, .omega = 0.0f },
    };

    const f32 pixels_per_meter = 400.0f;
    matf32 world_to_screen = diag_matf32(arena, 2, pixels_per_meter);
    MAT_GET(world_to_screen, 1, 1) *= -1.f;

    while (!window->should_close) {
        if (window->needs_redraw) {
            u64 now_ns = time_now_ns();
            acc_ns += MIN(now_ns - last_ns, MAX_FRAME_NS);
            last_ns = now_ns;
            while (acc_ns >= SIM_DT_NS) {
                pendulum_step(&pendulum, SIM_DT);
                acc_ns -= SIM_DT_NS;
            }

            framebuffer_t fb = window_framebuffer(window);
            fill_rect(fb, 0, 0, fb.width, fb.height, RGB(0x21, 0x21, 0x21));
            vec2f32 root = scale_v2f32(v2f32((f32)fb.width, (f32)fb.height), 0.5f);
            vec2f32 bob = add_v2f32(root, mul_matf32_v2f32(world_to_screen, pendulum_position(&pendulum)));
            draw_line(fb, root.x, root.y, bob.x, bob.y, 3.0f, RGB(0xCC, 0xCC, 0xCC));
            fill_circle(fb, root.x, root.y, 6.0f, RGB(0xCC, 0xCC, 0xCC));
            fill_circle(fb, bob.x, bob.y, 24.0f, RGB(0x88, 0x88, 0xFF));
            window_present(window);
        }
        window_wait_events(window);
    }
    window_destroy(window);
    return EXIT_SUCCESS;
}
