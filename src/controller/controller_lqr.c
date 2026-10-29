#pragma once

#include "controller/controller_lqr.h"
#include "math/math_inc.h"

#define LQR_N 4
#define LQR_MAX_ITERATIONS 2000000
#define LQR_TOLERANCE 1e-10

static void lqr_continuous_model(arena_t *arena, const pendulum_params_t *p, matf64 *a, matf64 *b)
{
    f64 M = p->cart_mass, m = p->bob_mass, L = p->length, g = -p->gravity.y, fr = p->cart_friction;
    const f64 a_values[LQR_N * LQR_N] = { 0,
                                          1,
                                          0,
                                          0,
                                          0,
                                          -fr / M,
                                          m * g / M,
                                          0,
                                          0,
                                          0,
                                          0,
                                          1,
                                          0,
                                          -fr / (M * L),
                                          (g / L) * (1.0 + m / M),
                                          0 };
    const f64 b_values[LQR_N] = { 0, 1.0 / M, 0, 1.0 / (M * L) };
    *a = from_array_matf64(arena, LQR_N, LQR_N, a_values);
    *b = from_array_matf64(arena, LQR_N, 1, b_values);
}

static void lqr_discretize(arena_t *arena, matf64 a, matf64 b, f64 dt, matf64 *ad, matf64 *bd)
{
    *ad = create_matf64(arena, LQR_N, LQR_N);
    *bd = create_matf64(arena, LQR_N, 1);

    arena_temp_t temp = arena_temp_begin(arena);
    matf64 m = create_matf64(arena, LQR_N + 1, LQR_N + 1);
    paste_matf64(m, 0, 0, a);
    paste_matf64(m, 0, LQR_N, b);
    scale_matf64(m, dt);
    matf64 e = expm_matf64(arena, m);
    assign_matf64(*ad, slice_matf64(arena, e, 0, 0, LQR_N, LQR_N));
    assign_matf64(*bd, slice_matf64(arena, e, 0, LQR_N, LQR_N, 1));
    arena_temp_end(temp);
}

u32 lqr_solve(arena_t *scratch, const pendulum_params_t *params, lqr_weights_t weights, f32 dt,
              f32 k_out[4])
{
    arena_temp_t temp = arena_temp_begin(scratch);

    matf64 a, b, ad, bd;
    lqr_continuous_model(scratch, params, &a, &b);
    lqr_discretize(scratch, a, b, dt, &ad, &bd);
    matf64 ad_t = transpose_matf64(scratch, ad);

    matf64 q = create_matf64(scratch, LQR_N, LQR_N);
    for (u16 i = 0; i < LQR_N; i++) {
        MAT_SET(q, i, i, weights.q[i]);
    }
    f64 r = weights.r;

    matf64 p = copy_matf64(scratch, q);
    matf64 k = create_matf64(scratch, 1, LQR_N);

    u32 result = 0;
    for (u32 iter = 1; iter <= LQR_MAX_ITERATIONS; iter++) {
        arena_temp_t step = arena_temp_begin(scratch);

        matf64 pb = mul_matf64(scratch, p, bd);
        matf64 pb_t = transpose_matf64(scratch, pb);
        f64 s = r + MAT_GET(mul_matf64(scratch, pb_t, bd), 0, 0);

        matf64 k_next = mul_matf64(scratch, pb_t, ad);
        scale_matf64(k_next, 1.0 / s);
        assign_matf64(k, k_next);

        matf64 pad = mul_matf64(scratch, p, ad);
        matf64 p_next = add_matf64(scratch, q, mul_matf64(scratch, ad_t, pad));
        matf64 kk = mul_matf64(scratch, transpose_matf64(scratch, k), k);
        scale_matf64(kk, s);
        p_next = sub_matf64(scratch, p_next, kk);

        symmetrize_matf64(p_next);

        f64 delta = max_abs_matf64(sub_matf64(scratch, p_next, p));
        f64 scale = max_abs_matf64(p_next);
        assign_matf64(p, p_next);
        arena_temp_end(step);

        if (delta <= LQR_TOLERANCE * MAX(scale, 1.0)) {
            result = iter;
            break;
        }
    }

    if (result != 0) {
        for (u16 i = 0; i < LQR_N; i++) {
            k_out[i] = (f32)MAT_GET(k, 0, i);
        }
    }
    arena_temp_end(temp);
    return result;
}
