// headers
#include "base/base_inc.h"
#include "math/math_inc.h"
#include "draw/draw_inc.h"
#include "window/window_inc.h"

// units
#include "base/base_inc.c"
#include "math/math_inc.c"
#include "draw/draw_inc.c"
#include "window/window_inc.c"

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

    while (!window->should_close) {
        if (window->needs_redraw) {
            framebuffer_t fb = window_framebuffer(window);
            fill_rect(fb, 0, 0, fb.width, fb.height, RGB(0x21, 0x21, 0x21));
            i32 rect_w = fb.width / 4;
            i32 rect_h = fb.height / 4;
            fill_rect(fb, (fb.width - rect_w) / 2, (fb.height - rect_h) / 2, rect_w, rect_h,
                      RGB(0x88, 0x88, 0xFF));
            window_present(window);
        }
        window_wait_events(window);
    }
    window_destroy(window);
    return EXIT_SUCCESS;
}
