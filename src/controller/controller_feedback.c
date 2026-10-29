#pragma once

#include "controller/controller_feedback.h"

static f32 controller_feedback_update(controller_t *self, pendulum_measurement_t m, f32 target_x,
                                      f32 dt)
{
    (void)dt;
    controller_feedback_t *c = CONTAINER_OF(self, controller_feedback_t, base);
    return -(c->k[0] * (m.x - target_x) + c->k[1] * m.v + c->k[2] * m.phi + c->k[3] * m.omega);
}

void controller_feedback_init(controller_feedback_t *c, const f32 k[4])
{
    c->base = (controller_t){ .update = controller_feedback_update, .reset = NULL };
    for (int i = 0; i < 4; i++) {
        c->k[i] = k[i];
    }
}
