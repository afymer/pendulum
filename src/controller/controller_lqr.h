#pragma once

#include "sim/sim_pendulum.h"

typedef struct lqr_weights_s lqr_weights_t;
struct lqr_weights_s {
    f32 q[4];
    f32 r;
};

u32 lqr_solve(arena_t *scratch, const pendulum_params_t *params, lqr_weights_t weights, f32 dt,
              f32 k_out[4]);
