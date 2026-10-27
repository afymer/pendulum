#include "ui_app.h"
#include <assert.h>
#include "draw/draw_rect.h"
#include "window/window_inc.h"

ui_app_t *ui_app_create(void)
{
    arena_t *arena = arena_create((arena_params_t){
        .reserve_size = UI_ARENA_SIZE,
        .commit_size = UI_ARENA_SIZE,
    });
    if (NULL == arena) {
        return NULL;
    }
    ui_app_t *app = PUSH_STRUCT(arena, ui_app_t);
    *app = (ui_app_t){ .arena = arena };

    app->frame_arena = arena_create((arena_params_t){
        .reserve_size = UI_ARENA_SIZE,
        .commit_size = UI_ARENA_SIZE,
    });
    app->entities = ui_slotmap_create(arena, UI_MAX_ENTITIES, UI_ENTITY_SIZE);
    app->views = ui_slotmap_create(arena, UI_MAX_VIEWS, sizeof(ui_view_t));
    return app;
}

void ui_app_destroy(ui_app_t *app)
{
    if (NULL == app) {
        return;
    }
    if (app->views) {
        for (u32 i = 0; i < app->views->capacity; i++) {
            if (app->views->occupied[i]) {
                ui_view_t *view = (ui_view_t *)(app->views->data + i * app->views->element_size);
                if (view->vtable && view->vtable->destroy) {
                    view->vtable->destroy(view->entity);
                }
                if (view->arena) {
                    arena_free(view->arena);
                }
            }
        }
    }
    for (u32 i = 0; i < app->window_count; i++) {
        window_destroy(app->windows[i]);
    }
    app->window_count = 0;
    if (app->frame_arena) {
        arena_free(app->frame_arena);
    }
    arena_free(app->arena);
}

void ui_app_add_window(ui_app_t *app, window_t *window)
{
    if (app->window_count < UI_MAX_WINDOWS) {
        app->windows[app->window_count++] = window;
    }
}

ui_handle_t ui_view_create(ui_app_t *app, void *entity, const ui_view_vtable_t *vtable)
{
    arena_t *arena = arena_create((arena_params_t){
        .reserve_size = UI_VIEW_ARENA_SIZE,
        .commit_size = UI_VIEW_COMMIT_SIZE,
    });
    ui_view_t view = {
        .entity = entity,
        .vtable = vtable,
        .arena = arena,
        .is_dirty = true,
        .dirty_buffers = LINUX_WAYLAND_BUFFER_COUNT,
    };
    return ui_slotmap_insert(app->views, &view);
}

void ui_view_destroy(ui_app_t *app, ui_handle_t handle)
{
    ui_view_t *view = ui_slotmap_get(app->views, handle);
    if (view) {
        if (view->vtable && view->vtable->destroy) {
            view->vtable->destroy(view->entity);
        }
        if (view->arena) {
            arena_free(view->arena);
        }
        ui_slotmap_erase(app->views, handle);
    }
}

void ui_notify(ui_app_t *app, ui_handle_t handle)
{
    ui_view_t *view = ui_slotmap_get(app->views, handle);
    if (view) {
        view->is_dirty = true;
        view->dirty_buffers = LINUX_WAYLAND_BUFFER_COUNT;
    }
    for (u32 i = 0; i < app->window_count; i++) {
        app->windows[i]->is_dirty = true;
    }
}

static void ui_link_child(ui_element_t *parent, ui_element_t *child)
{
    assert(parent->kind == UI_NODE_DIV);
    child->parent = parent;
    if (parent->div.last_child) {
        parent->div.last_child->next_sibling = child;
    } else {
        parent->div.first_child = child;
    }
    parent->div.last_child = child;
}

ui_element_t *ui_div_begin(ui_cx_t *cx, ui_div_params_t params)
{
    arena_t *arena = cx->arena ? cx->arena : cx->app->frame_arena;
    ui_element_t *el = PUSH_STRUCT(arena, ui_element_t);
    *el = (ui_element_t){
        .kind = UI_NODE_DIV,
        .style = params.style,
        .view_handle = cx->current_view,
        .div = {
            .background = params.background,
        },
    };

    if (cx->parent_depth > 0) {
        ui_element_t *parent = cx->parent_stack[cx->parent_depth - 1];
        ui_link_child(parent, el);
    } else {
        assert(NULL == cx->root);
        cx->root = el;
    }
    assert(cx->parent_depth < UI_MAX_PARENT_DEPTH);
    cx->parent_stack[cx->parent_depth++] = el;
    return el;
}

void ui_div_end(ui_cx_t *cx)
{
    if (cx->parent_depth > 0) {
        cx->parent_depth--;
    }
}

ui_element_t *ui_div(ui_cx_t *cx, ui_div_params_t params)
{
    ui_element_t *el = ui_div_begin(cx, params);
    ui_div_end(cx);
    return el;
}

ui_element_t *ui_canvas(ui_cx_t *cx, ui_canvas_params_t params)
{
    arena_t *arena = cx->arena ? cx->arena : cx->app->frame_arena;
    ui_element_t *el = PUSH_STRUCT(arena, ui_element_t);
    *el = (ui_element_t){
        .kind = UI_NODE_CANVAS,
        .style = params.style,
        .view_handle = cx->current_view,
        .canvas = {
            .draw = params.draw,
            .user_data = params.user_data,
            .background = params.background,
        },
    };

    if (cx->parent_depth > 0) {
        ui_element_t *parent = cx->parent_stack[cx->parent_depth - 1];
        ui_link_child(parent, el);
    } else {
        assert(NULL == cx->root);
        cx->root = el;
    }
    return el;
}

static void ui_view_render_into_container(ui_app_t *app, window_t *window, ui_view_t *view,
                                          ui_element_t *container, ui_handle_t view_handle)
{
    if (view->is_dirty || NULL == view->root_element) {
        view->arena->opl = ARENA_METADATA_SIZE;
        container->div.first_child = NULL;
        container->div.last_child = NULL;
        ui_cx_t view_cx = {
            .app = app,
            .window = window,
            .arena = view->arena,
            .current_view = view_handle,
            .parent_stack = { container },
            .parent_depth = 1,
        };
        if (view->vtable && view->vtable->render) {
            view->vtable->render(view->entity, &view_cx);
        }
        view->root_element = container->div.first_child;
        view->is_dirty = false;
    } else {
        container->div.first_child = view->root_element;
        container->div.last_child = view->root_element;
        if (view->root_element) {
            view->root_element->parent = container;
        }
    }
}

ui_element_t *ui_view(ui_cx_t *cx, ui_handle_t view_handle, layout_style_t style)
{
    ui_view_t *view = ui_slotmap_get(cx->app->views, view_handle);
    if (NULL == view) {
        return NULL;
    }

    ui_element_t *container = ui_div_begin(cx, (ui_div_params_t){ .style = style });
    container->view_handle = view_handle;
    view->container = container;

    ui_view_render_into_container(cx->app, cx->window, view, container, view_handle);

    ui_div_end(cx);
    return container;
}

ui_element_t *ui_push_container(ui_cx_t *cx, layout_style_t style, u32 background)
{
    return ui_div_begin(cx, (ui_div_params_t){ .style = style, .background = background });
}

void ui_pop(ui_cx_t *cx)
{
    ui_div_end(cx);
}

ui_element_t *ui_append_child(ui_cx_t *cx, layout_style_t style, u32 background)
{
    return ui_div(cx, (ui_div_params_t){ .style = style, .background = background });
}

ui_element_t *ui_append_view(ui_cx_t *cx, ui_handle_t view_handle, layout_style_t style)
{
    return ui_view(cx, view_handle, style);
}

static void ui_layout_fit_sizes(ui_element_t *node)
{
    bool vertical = node->style.axis == LAYOUT_AXIS_VERTICAL;
    f32 main_total = 0.0f;
    f32 cross_max = 0.0f;
    u32 count = 0;

    ui_element_t *first = (node->kind == UI_NODE_DIV) ? node->div.first_child : NULL;
    for (ui_element_t *c = first; c; c = c->next_sibling) {
        ui_layout_fit_sizes(c);
        main_total += vertical ? c->computed_size.y : c->computed_size.x;
        cross_max = MAX(cross_max, vertical ? c->computed_size.x : c->computed_size.y);
        count++;
    }
    if (count > 1) {
        main_total += node->style.gap * (f32)(count - 1);
    }

    if (0 == count) {
        node->computed_size = v2f32(node->style.width, node->style.height);
        return;
    }
    f32 inset = 2.0f * node->style.padding;
    f32 w = (vertical ? cross_max : main_total) + inset;
    f32 h = (vertical ? main_total : cross_max) + inset;
    if (node->style.width > 0.0f) {
        w = node->style.width;
    }
    if (node->style.height > 0.0f) {
        h = node->style.height;
    }
    node->computed_size = v2f32(w, h);
}

static void ui_layout_place(ui_element_t *node)
{
    if (node->kind != UI_NODE_DIV) {
        return;
    }

    bool vertical = node->style.axis == LAYOUT_AXIS_VERTICAL;
    f32 parent_main = vertical ? node->computed_size.y : node->computed_size.x;
    f32 parent_cross = vertical ? node->computed_size.x : node->computed_size.y;
    f32 inner_main = MAX(0.0f, parent_main - 2.0f * node->style.padding);
    f32 inner_cross = MAX(0.0f, parent_cross - 2.0f * node->style.padding);

    f32 fixed_main = 0.0f;
    f32 total_grow = 0.0f;
    u32 count = 0;

    for (ui_element_t *c = node->div.first_child; c; c = c->next_sibling) {
        if (c->style.grow > 0.0f) {
            total_grow += c->style.grow;
        } else {
            fixed_main += vertical ? c->computed_size.y : c->computed_size.x;
        }
        count++;
    }
    if (count > 1) {
        fixed_main += node->style.gap * (f32)(count - 1);
    }

    f32 remaining_main = MAX(0.0f, inner_main - fixed_main);

    for (ui_element_t *c = node->div.first_child; c; c = c->next_sibling) {
        if (c->style.grow > 0.0f && total_grow > 0.0f) {
            f32 flex_size = remaining_main * (c->style.grow / total_grow);
            if (vertical) {
                c->computed_size.y = flex_size;
            } else {
                c->computed_size.x = flex_size;
            }
        }
        if (vertical) {
            if (c->style.width <= 0.0f) {
                c->computed_size.x = inner_cross;
            }
        } else {
            if (c->style.height <= 0.0f) {
                c->computed_size.y = inner_cross;
            }
        }
    }

    vec2f32 cursor = v2f32(node->computed_pos.x + node->style.padding,
                           node->computed_pos.y + node->style.padding);

    for (ui_element_t *c = node->div.first_child; c; c = c->next_sibling) {
        c->computed_pos = cursor;
        ui_layout_place(c);
        if (vertical) {
            cursor.y += c->computed_size.y + node->style.gap;
        } else {
            cursor.x += c->computed_size.x + node->style.gap;
        }
    }
}

void ui_layout_solve(ui_element_t *root, f32 width, f32 height)
{
    ui_layout_fit_sizes(root);
    root->computed_size = v2f32(width, height);
    root->computed_pos = v2f32(0.0f, 0.0f);
    ui_layout_place(root);
}

static void ui_view_update_dirty_views(ui_app_t *app, window_t *win, ui_handle_t root_handle,
                                       bool *need_layout)
{
    for (u32 i = 0; i < app->views->capacity; i++) {
        if (!app->views->occupied[i]) {
            continue;
        }
        ui_handle_t handle = { .index = i, .gen = app->views->generations[i] };
        if (ui_handle_eq(handle, root_handle)) {
            continue;
        }
        ui_view_t *view = (ui_view_t *)(app->views->data + i * app->views->element_size);
        if (view->is_dirty && view->container) {
            ui_element_t *old_root = view->root_element;
            vec2f32 old_size = old_root ? old_root->computed_size : v2f32(0.0f, 0.0f);

            ui_view_render_into_container(app, win, view, view->container, handle);

            if (view->root_element) {
                view->root_element->computed_pos = view->container->computed_pos;
                view->root_element->computed_size = view->container->computed_size;
                if (view->root_element->kind == UI_NODE_DIV &&
                    view->root_element->div.first_child) {
                    ui_layout_solve(view->root_element, view->container->computed_size.x,
                                    view->container->computed_size.y);
                }
            }

            if (old_root && (old_size.x != view->container->computed_size.x ||
                             old_size.y != view->container->computed_size.y)) {
                *need_layout = true;
            }
        }
    }
}

static inline framebuffer_t ui_sub_framebuffer(framebuffer_t fb, i32 x, i32 y, i32 w, i32 h)
{
    i32 x0 = MAX(x, 0);
    i32 y0 = MAX(y, 0);
    i32 x1 = MIN(x + w, fb.width);
    i32 y1 = MIN(y + h, fb.height);
    i32 cw = MAX(0, x1 - x0);
    i32 ch = MAX(0, y1 - y0);
    return (framebuffer_t){
        .pixels = (u32 *)((u8 *)fb.pixels + (usize)y0 * (usize)fb.stride) + x0,
        .width = cw,
        .height = ch,
        .stride = fb.stride,
    };
}

static void ui_element_paint_node(ui_cx_t *cx, ui_element_t *el, bool force_redraw)
{
    if (NULL == el) {
        return;
    }
    ui_view_t *view = ui_slotmap_get(cx->app->views, el->view_handle);
    bool view_dirty = (view && view->dirty_buffers > 0);
    bool should_paint = force_redraw || el->is_dirty || view_dirty;

    if (should_paint) {
        if (el->kind == UI_NODE_DIV && el->div.background) {
            if (cx->fb.pixels) {
                fill_rect(cx->fb, (i32)el->computed_pos.x, (i32)el->computed_pos.y,
                          (i32)el->computed_size.x, (i32)el->computed_size.y, el->div.background);
            }
            window_damage(cx->window, (i32)el->computed_pos.x, (i32)el->computed_pos.y,
                          (i32)el->computed_size.x, (i32)el->computed_size.y);
        } else if (el->kind == UI_NODE_CANVAS) {
            if (cx->fb.pixels && el->canvas.background) {
                fill_rect(cx->fb, (i32)el->computed_pos.x, (i32)el->computed_pos.y,
                          (i32)el->computed_size.x, (i32)el->computed_size.y,
                          el->canvas.background);
            }
            if (el->canvas.draw) {
                framebuffer_t prev_fb = cx->fb;
                cx->fb =
                    ui_sub_framebuffer(prev_fb, (i32)el->computed_pos.x, (i32)el->computed_pos.y,
                                       (i32)el->computed_size.x, (i32)el->computed_size.y);
                el->canvas.draw(cx->fb, el->canvas.user_data);
                cx->fb = prev_fb;
            }
            window_damage(cx->window, (i32)el->computed_pos.x, (i32)el->computed_pos.y,
                          (i32)el->computed_size.x, (i32)el->computed_size.y);
        }
    }

    if (el->kind == UI_NODE_DIV) {
        for (ui_element_t *child = el->div.first_child; child; child = child->next_sibling) {
            ui_element_paint_node(cx, child, force_redraw);
        }
    }
}

void ui_element_paint(ui_cx_t *cx, ui_element_t *root)
{
    if (NULL == cx || NULL == cx->window || NULL == root) {
        return;
    }
    cx->fb = window_framebuffer(cx->window);
    bool force = (cx->window->full_redraw_buffers > 0) || cx->window->needs_full_redraw;
    if (force) {
        window_damage(cx->window, 0, 0, cx->window->width, cx->window->height);
    }
    ui_element_paint_node(cx, root, force);
    if (cx->window->full_redraw_buffers > 0) {
        cx->window->full_redraw_buffers--;
    }
    cx->window->needs_full_redraw = (cx->window->full_redraw_buffers > 0);
    window_present(cx->window);

    for (u32 i = 0; i < cx->app->views->capacity; i++) {
        if (cx->app->views->occupied[i]) {
            ui_view_t *v = (ui_view_t *)(cx->app->views->data + i * cx->app->views->element_size);
            if (v->dirty_buffers > 0) {
                v->dirty_buffers--;
            }
        }
    }
}

void ui_window_render(ui_app_t *app, window_t *win)
{
    if (!win->is_dirty) {
        return;
    }

    ui_view_t *root_view = ui_slotmap_get(app->views, win->root_view);
    if (NULL == root_view) {
        return;
    }

    bool need_layout = (NULL == root_view->root_element);

    if (root_view->is_dirty || NULL == root_view->root_element) {
        root_view->arena->opl = ARENA_METADATA_SIZE;
        root_view->root_element = NULL;
        ui_cx_t cx = {
            .app = app,
            .window = win,
            .arena = root_view->arena,
            .current_view = win->root_view,
        };
        if (root_view->vtable && root_view->vtable->render) {
            root_view->vtable->render(root_view->entity, &cx);
        }
        root_view->root_element = cx.root;
        root_view->is_dirty = false;
        need_layout = true;
    } else {
        ui_view_update_dirty_views(app, win, win->root_view, &need_layout);
    }

    if ((need_layout || win->needs_full_redraw) && root_view->root_element) {
        ui_layout_solve(root_view->root_element, (f32)win->width, (f32)win->height);
    }

    ui_cx_t paint_cx = {
        .app = app,
        .window = win,
    };
    ui_element_paint(&paint_cx, root_view->root_element);
    win->is_dirty = false;
}
