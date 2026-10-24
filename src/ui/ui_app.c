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
    arena_free(app->frame_arena);
    arena_free(app->arena);
}

void ui_app_add_window(ui_app_t *app, window_t *window)
{
    if (app->window_count < UI_MAX_WINDOWS) {
        app->windows[app->window_count++] = window;
    }
}

void ui_notify(ui_app_t *app, ui_handle_t handle)
{
    (void)handle;
    for (u32 i = 0; i < app->window_count; i++) {
        app->windows[i]->is_dirty = true;
    }
}

ui_element_t *ui_append_child(ui_cx_t *cx, layout_style_t style, u32 background)
{
    ui_element_t *el = PUSH_STRUCT(cx->app->frame_arena, ui_element_t);
    *el = (ui_element_t){
        .kind = UI_ELEMENT_CONTAINER,
        .style = style,
        .background = background,
    };

    if (cx->parent_depth > 0) {
        ui_element_t *parent = cx->parent_stack[cx->parent_depth - 1];
        el->parent = parent;
        if (parent->last_child) {
            parent->last_child->next_sibling = el;
        } else {
            parent->first_child = el;
        }
        parent->last_child = el;
    } else {
        assert(NULL == cx->root);
        cx->root = el;
    }
    return el;
}

ui_element_t *ui_push_container(ui_cx_t *cx, layout_style_t style, u32 background)
{
    ui_element_t *el = ui_append_child(cx, style, background);
    assert(cx->parent_depth < UI_MAX_PARENT_DEPTH);
    cx->parent_stack[cx->parent_depth++] = el;
    return el;
}

void ui_pop(ui_cx_t *cx)
{
    if (cx->parent_depth > 0) {
        cx->parent_depth--;
    }
}

static void ui_layout_fit_sizes(ui_element_t *node)
{
    bool vertical = node->style.axis == LAYOUT_AXIS_VERTICAL;
    f32 main_total = 0.0f;
    f32 cross_max = 0.0f;
    u32 count = 0;

    for (ui_element_t *c = node->first_child; c; c = c->next_sibling) {
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
    node->computed_size.x = (vertical ? cross_max : main_total) + inset;
    node->computed_size.y = (vertical ? main_total : cross_max) + inset;
}

static void ui_layout_place(ui_element_t *node)
{
    bool vertical = node->style.axis == LAYOUT_AXIS_VERTICAL;
    vec2f32 cursor = v2f32(node->computed_pos.x + node->style.padding,
                           node->computed_pos.y + node->style.padding);

    for (ui_element_t *c = node->first_child; c; c = c->next_sibling) {
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

static void ui_element_paint_node(ui_cx_t *cx, ui_element_t *el, framebuffer_t fb)
{
    if (NULL == el) {
        return;
    }
    if (fb.pixels && el->background) {
        fill_rect(fb, (i32)el->computed_pos.x, (i32)el->computed_pos.y, (i32)el->computed_size.x,
                  (i32)el->computed_size.y, el->background);
    }
    if (el->custom_draw) {
        el->custom_draw(el, cx);
    }
    for (ui_element_t *child = el->first_child; child; child = child->next_sibling) {
        ui_element_paint_node(cx, child, fb);
    }
}

void ui_element_paint(ui_cx_t *cx, ui_element_t *root)
{
    if (NULL == cx || NULL == cx->window || NULL == root) {
        return;
    }
    framebuffer_t fb = window_framebuffer(cx->window);
    ui_element_paint_node(cx, root, fb);
    window_present(cx->window);
}

void ui_window_render(ui_app_t *app, window_t *win)
{
    if (!win->is_dirty) {
        return;
    }

    arena_temp_t scratch = arena_temp_begin(app->frame_arena);

    ui_cx_t cx = { .app = app, .window = win };

    ui_view_t *view = ui_slotmap_get(app->views, win->root_view);
    if (view && view->vtable->render) {
        view->vtable->render(view->entity, &cx);
    }

    if (cx.root) {
        ui_layout_solve(cx.root, (f32)win->width, (f32)win->height);
        ui_element_paint(&cx, cx.root);
    }

    arena_temp_end(scratch);
    win->is_dirty = false;
}
