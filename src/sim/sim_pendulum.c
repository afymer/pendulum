#pragma once

#include "sim/sim_pendulum.h"

#define SIM_PI 3.14159265358979323846f

static f32 wrap_angle(f32 a)
{
    a = fmodf(a + SIM_PI, 2.0f * SIM_PI);
    if (a <= 0.0f) {
        a += 2.0f * SIM_PI;
    }
    return a - SIM_PI;
}

vec2f32 pendulum_position(const pendulum_t *pendulum)
{
    f32 theta = pendulum->state.theta;
    vec2f32 rod = scale_v2f32(v2f32(sinf(theta), -cosf(theta)), pendulum->params.length);
    return add_v2f32(v2f32(pendulum->state.x, 0.0f), rod);
}

pendulum_measurement_t pendulum_measure(const pendulum_t *pendulum)
{
    const pendulum_state_t *s = &pendulum->state;
    return (pendulum_measurement_t){
        .x = s->x,
        .v = s->v,
        .phi = wrap_angle(s->theta - SIM_PI),
        .omega = s->omega,
    };
}

static pendulum_state_t pendulum_deriv(const pendulum_params_t *p, pendulum_state_t s, f32 force)
{
    f32 g = -p->gravity.y;
    f32 sin_t = sinf(s.theta);
    f32 cos_t = cosf(s.theta);
    f32 m = p->bob_mass;

    f32 f = force - p->cart_friction * s.v;
    f32 x_acc = (f + m * p->length * s.omega * s.omega * sin_t + m * g * sin_t * cos_t) /
                (p->cart_mass + m * sin_t * sin_t);
    f32 theta_acc = (-g * sin_t - cos_t * x_acc) / p->length;

    return (pendulum_state_t){ .x = s.v, .v = x_acc, .theta = s.omega, .omega = theta_acc };
}

static pendulum_state_t pendulum_state_axpy(pendulum_state_t s, pendulum_state_t d, f32 k)
{
    return (pendulum_state_t){
        .x = s.x + k * d.x,
        .v = s.v + k * d.v,
        .theta = s.theta + k * d.theta,
        .omega = s.omega + k * d.omega,
    };
}

void pendulum_step(pendulum_t *pendulum, f32 force, f32 dt)
{
    const pendulum_params_t *p = &pendulum->params;
    pendulum_state_t s = pendulum->state;
    force = CLAMP(force, -p->max_force, p->max_force);

    pendulum_state_t k1 = pendulum_deriv(p, s, force);
    pendulum_state_t k2 = pendulum_deriv(p, pendulum_state_axpy(s, k1, 0.5f * dt), force);
    pendulum_state_t k3 = pendulum_deriv(p, pendulum_state_axpy(s, k2, 0.5f * dt), force);
    pendulum_state_t k4 = pendulum_deriv(p, pendulum_state_axpy(s, k3, dt), force);

    pendulum_state_t *out = &pendulum->state;
    out->x = s.x + dt / 6.0f * (k1.x + 2.0f * k2.x + 2.0f * k3.x + k4.x);
    out->v = s.v + dt / 6.0f * (k1.v + 2.0f * k2.v + 2.0f * k3.v + k4.v);
    out->theta = s.theta + dt / 6.0f * (k1.theta + 2.0f * k2.theta + 2.0f * k3.theta + k4.theta);
    out->omega = s.omega + dt / 6.0f * (k1.omega + 2.0f * k2.omega + 2.0f * k3.omega + k4.omega);
    out->theta = wrap_angle(out->theta);
}

f32 pendulum_energy(const pendulum_t *pendulum)
{
    pendulum_params_t p = pendulum->params;
    f32 g = -p.gravity.y;
    f32 v = p.length * pendulum->state.omega;
    f32 height = p.length * (1.0f - cosf(pendulum->state.theta));
    return 0.5f * p.bob_mass * v * v + p.bob_mass * g * height;
}
