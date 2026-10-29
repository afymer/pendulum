#include "pendulum_sim.h"
#include "math/math_inc.h"

bool pendulum_sim_init(pendulum_sim_t *sim)
{
    *sim = (pendulum_sim_t){
        .pendulum = {
            .params = {
                .length = 0.6f,
                .bob_mass = 0.1f,
                .cart_mass = 0.5f,
                .gravity = v2f32(0.0f, -9.81f),
                .cart_friction = 0.1f,
                .max_force = 100.0f,
            },
            .state = { .theta = 3.1f },
        },
    };

    const lqr_weights_t weights = {
        .q = { 10.0f, 1.0f, 100.0f, 1.0f },
        .r = 0.1f,
    };
    f32 gains[4];
    arena_t *scratch_arena = arena_create((arena_params_t){
        .reserve_size = 64 * 1024,
        .commit_size = 64 * 1024,
    });
    if (NULL == scratch_arena) {
        LOG_ERROR("cannot allocate scratch arena for LQR");
        return false;
    }

    u32 design_iterations =
        lqr_solve(scratch_arena, &sim->pendulum.params, weights, CONTROL_DT, gains);
    arena_free(scratch_arena);

    if (0 == design_iterations) {
        LOG_ERROR("LQR did not converge");
        return false;
    }

    controller_feedback_init(&sim->feedback, gains);
    return true;
}

void pendulum_sim_tick(pendulum_sim_t *sim)
{
    sim->target_x = fmodf(sim->sim_time, 2.0f * TARGET_SWITCH_S) < TARGET_SWITCH_S ? 1.0f : -1.0f;

    if (sim->sim_ticks % CONTROL_DIVIDER == 0) {
        sim->force = controller_update(&sim->feedback.base, pendulum_measure(&sim->pendulum),
                                       sim->target_x, CONTROL_DT);
    }

    pendulum_step(&sim->pendulum, sim->force, SIM_DT);
    sim->sim_ticks++;
    sim->sim_time += SIM_DT;
}
