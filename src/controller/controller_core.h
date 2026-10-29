#pragma once

#include "sim/sim_pendulum.h"

typedef struct controller_s controller_t;
struct controller_s {
    f32 (*update)(controller_t *self, pendulum_measurement_t m, f32 target_x, f32 dt);
    void (*reset)(controller_t *self);
};

static inline f32 controller_update(controller_t *c, pendulum_measurement_t m, f32 target_x,
                                    f32 dt)
{
    return c->update(c, m, target_x, dt);
}

static inline void controller_reset(controller_t *c)
{
    if (c->reset) {
        c->reset(c);
    }
}
