#pragma once

typedef enum ui_event_kind_e {
    UI_EVENT_NONE = 0,
    UI_EVENT_WINDOW_CLOSE,
    UI_EVENT_WINDOW_RESIZE,
    UI_EVENT_WINDOW_REDRAW,
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_MOUSE_BUTTON,
    UI_EVENT_MOUSE_SCROLL,
    UI_EVENT_KEY,
} ui_event_kind_t;

typedef struct ui_event_s ui_event_t;
struct ui_event_s {
    ui_event_kind_t kind;
    union {
        struct {
            i32 width;
            i32 height;
        } resize;
        struct {
            f32 x;
            f32 y;
            u32 button;
            bool is_down;
        } mouse;
        struct {
            f32 dx;
            f32 dy;
        } scroll;
        struct {
            u32 key;
            bool is_down;
            u32 modifiers;
        } key;
    };
};
