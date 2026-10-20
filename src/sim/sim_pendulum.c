#include "sim/sim_pendulum.h"

vec2f32 pendulum_position(const pendulum_t *pendulum)
{
    f32 theta = pendulum->state.theta;
    return scale_v2f32(v2f32(sinf(theta), -cosf(theta)), pendulum->params.length);
}

void pendulum_step(pendulum_t *pendulum, f32 dt)
{
    pendulum_params_t p = pendulum->params;
    pendulum_state_t *s = &pendulum->state;

    vec2f32 r = pendulum_position(pendulum);
    f32 alpha = cross_v2f32(r, p.gravity) / (p.length * p.length);

    s->omega += alpha * dt;
    s->theta += s->omega * dt;
}

f32 pendulum_energy(const pendulum_t *pendulum)
{
    pendulum_params_t p = pendulum->params;
    f32 v = p.length * pendulum->state.omega;

    vec2f32 bottom = v2f32(0.0f, -p.length);
    f32 potential = -dot_v2f32(p.gravity, sub_v2f32(pendulum_position(pendulum), bottom));
    return 0.5f * v * v + potential;
}
