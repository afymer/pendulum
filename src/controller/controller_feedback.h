#pragma once

#include "controller_core.h"

typedef struct controller_feedback_s controller_feedback_t;
struct controller_feedback_s {
    controller_t base;
    f32 k[4];
};

void controller_feedback_init(controller_feedback_t *c, const f32 k[4]);
