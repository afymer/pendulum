#include "pendulum_canvas_view.h"
#include "draw/draw_inc.h"

static void pendulum_canvas_draw(framebuffer_t fb, void *user_data)
{
    pendulum_canvas_view_t *view = user_data;
    if (NULL == view || NULL == view->sim) {
        return;
    }

    const pendulum_t *pendulum = &view->sim->pendulum;
    f32 ppm = view->pixels_per_meter > 0.0f ? view->pixels_per_meter : 350.0f;

    vec2f32 origin = v2f32((f32)fb.width * 0.5f, (f32)fb.height * 0.5f);
    vec2f32 root = v2f32(origin.x + pendulum->state.x * ppm, origin.y);
    vec2f32 rel = pendulum_position(pendulum);
    vec2f32 bob = v2f32(origin.x + rel.x * ppm, origin.y - rel.y * ppm);
    vec2f32 target = v2f32(origin.x + view->sim->target_x * ppm, origin.y);

    draw_line(fb, 0.0f, origin.y, (f32)fb.width, origin.y, 2.0f, 0xFF555555);
    fill_rect(fb, (i32)target.x - 2, (i32)origin.y - 20, 4, 40, 0xFF66AA66);
    fill_rect(fb, (i32)root.x - 50, (i32)root.y - 15, 100, 30, 0xFF999999);
    draw_line(fb, root.x, root.y, bob.x, bob.y, 3.0f, 0xFFCCCCCC);
    fill_circle(fb, bob.x, bob.y, 8.0f, 0xFF8888FF);
}

static void pendulum_canvas_render(void *self, ui_cx_t *cx)
{
    pendulum_canvas_view_t *view = self;
    ui_canvas(cx, (ui_canvas_params_t){
                      .style = { .grow = 1.0f },
                      .background = 0xFF212121,
                      .draw = pendulum_canvas_draw,
                      .user_data = view,
                  });
}

static const ui_view_vtable_t pendulum_canvas_view_vtable = {
    .render = pendulum_canvas_render,
    .destroy = NULL,
};

ui_handle_t pendulum_canvas_view_create(ui_app_t *app, const pendulum_sim_t *sim,
                                        f32 pixels_per_meter)
{
    pendulum_canvas_view_t *view = PUSH_STRUCT(app->arena, pendulum_canvas_view_t);
    *view = (pendulum_canvas_view_t){
        .sim = sim,
        .pixels_per_meter = pixels_per_meter,
    };
    return ui_view_create(app, view, &pendulum_canvas_view_vtable);
}
