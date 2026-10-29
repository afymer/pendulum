#pragma once

#include "math/math_vec.h"

typedef struct pendulum_params_s pendulum_params_t;
struct pendulum_params_s {
    f32 length;
    f32 bob_mass;
    f32 cart_mass;
    vec2f32 gravity;
    f32 cart_friction;
    f32 max_force;
};

typedef struct pendulum_state_s pendulum_state_t;
struct pendulum_state_s {
    f32 x;
    f32 v;
    f32 theta;
    f32 omega;
};

typedef struct pendulum_s pendulum_t;
struct pendulum_s {
    pendulum_params_t params;
    pendulum_state_t state;
};

typedef struct pendulum_measurement_s pendulum_measurement_t;
struct pendulum_measurement_s {
    f32 x;
    f32 v;
    f32 phi;
    f32 omega;
};

void pendulum_step(pendulum_t *pendulum, f32 force, f32 dt);
pendulum_measurement_t pendulum_measure(const pendulum_t *pendulum);
f32 pendulum_energy(const pendulum_t *pendulum);
vec2f32 pendulum_position(const pendulum_t *pendulum);
