#pragma once

#include "base/base_inc.h"
#include "controller/controller_inc.h"
#include "sim/sim_inc.h"

#define SIM_DT_NS (1 * NS_PER_MS)
#define SIM_DT ((f32)SIM_DT_NS / (f32)NS_PER_SEC)
#define CONTROL_DIVIDER 5
#define CONTROL_DT (SIM_DT * CONTROL_DIVIDER)
#define TARGET_SWITCH_S 6.0f
#define MAX_FRAME_NS (250 * NS_PER_MS)

typedef struct pendulum_sim_s {
    pendulum_t pendulum;
    controller_feedback_t feedback;
    u64 sim_ticks;
    f32 sim_time;
    f32 target_x;
    f32 force;
} pendulum_sim_t;

bool pendulum_sim_init(pendulum_sim_t *sim);
void pendulum_sim_tick(pendulum_sim_t *sim);
