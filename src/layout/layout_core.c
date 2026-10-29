#include "layout_core.h"

layout_node_t *layout_node_push(arena_t *arena, layout_node_t *parent, layout_style_t style)
{
    layout_node_t *node = PUSH_STRUCT(arena, layout_node_t);
    *node = (layout_node_t){ .style = style, .parent = parent };
    if (parent) {
        if (parent->last_child) {
            parent->last_child->next_sibling = node;
        } else {
            parent->first_child = node;
        }
        parent->last_child = node;
    }
    return node;
}

static void layout_fit_sizes(layout_node_t *node)
{
    bool vertical = node->style.axis == LAYOUT_AXIS_VERTICAL;
    f32 main_total = 0.0f;
    f32 cross_max = 0.0f;
    u32 count = 0;

    for (layout_node_t *c = node->first_child; c; c = c->next_sibling) {
        layout_fit_sizes(c);
        main_total += vertical ? c->size.y : c->size.x;
        cross_max = MAX(cross_max, vertical ? c->size.x : c->size.y);
        count++;
    }
    if (count > 1) {
        main_total += node->style.gap * (f32)(count - 1);
    }

    if (0 == count) {
        node->size = v2f32(node->style.width, node->style.height);
        return;
    }
    f32 inset = 2.0f * node->style.padding;
    node->size.x = (vertical ? cross_max : main_total) + inset;
    node->size.y = (vertical ? main_total : cross_max) + inset;
}

static void layout_place(layout_node_t *node)
{
    bool vertical = node->style.axis == LAYOUT_AXIS_VERTICAL;
    vec2f32 cursor = v2f32(node->pos.x + node->style.padding, node->pos.y + node->style.padding);

    for (layout_node_t *c = node->first_child; c; c = c->next_sibling) {
        c->pos = cursor;
        layout_place(c);
        if (vertical) {
            cursor.y += c->size.y + node->style.gap;
        } else {
            cursor.x += c->size.x + node->style.gap;
        }
    }
}

void layout_solve(layout_node_t *root, f32 width, f32 height)
{
    layout_fit_sizes(root);
    root->size = v2f32(width, height);
    root->pos = v2f32(0.0f, 0.0f);
    layout_place(root);
}
