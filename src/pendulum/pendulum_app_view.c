#include "pendulum_app_view.h"

static void pendulum_app_render(void *self, ui_cx_t *cx)
{
    pendulum_app_view_t *view = self;
    ui_div_begin(cx, (ui_div_params_t){
                         .style = {
                             .axis = LAYOUT_AXIS_HORIZONTAL,
                             .gap = 8.0f,
                             .padding = 8.0f,
                         },
                         .background = 0xFF18181B,
                     });
    {
        ui_div(cx, (ui_div_params_t){ .style = { .width = 220.0f }, .background = 0xFF212121 });
        ui_view(cx, view->canvas_view, (layout_style_t){ .grow = 1.0f });
        ui_div(cx, (ui_div_params_t){ .style = { .width = 260.0f }, .background = 0xFF212121 });
    }
    ui_div_end(cx);
}

static const ui_view_vtable_t pendulum_app_view_vtable = {
    .render = pendulum_app_render,
    .destroy = NULL,
};

ui_handle_t pendulum_app_view_create(ui_app_t *app, ui_handle_t canvas_view)
{
    pendulum_app_view_t *view = PUSH_STRUCT(app->arena, pendulum_app_view_t);
    *view = (pendulum_app_view_t){
        .canvas_view = canvas_view,
    };
    return ui_view_create(app, view, &pendulum_app_view_vtable);
}
