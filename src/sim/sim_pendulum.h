#pragma once

#include "math/math_vec.h"

typedef struct pendulum_params_s pendulum_params_t;
struct pendulum_params_s {
    f32 length;
    vec2f32 gravity;
};

typedef struct pendulum_state_s pendulum_state_t;
struct pendulum_state_s {
    f32 theta;
    f32 omega;
};

typedef struct pendulum_s pendulum_t;
struct pendulum_s {
    pendulum_params_t params;
    pendulum_state_t state;
};

void pendulum_step(pendulum_t *pendulum, f32 dt);
f32 pendulum_energy(const pendulum_t *pendulum);
vec2f32 pendulum_position(const pendulum_t *pendulum);
