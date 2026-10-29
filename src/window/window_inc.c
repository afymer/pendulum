#include "window_inc.h"

bool window_is_open(const window_t *window)
{
    return NULL != window && window->is_open;
}

void window_set_animating(window_t *window, bool animating)
{
    if (window && window->is_animating != animating) {
        window->is_animating = animating;
        if (animating) {
            window->is_dirty = true;
        }
    }
}

void dispatch_event(ui_app_t *app, window_t *window, const ui_event_t *event)
{
    (void)app;
    if (NULL == window || NULL == event) {
        return;
    }

    switch (event->kind) {
    case UI_EVENT_WINDOW_CLOSE: {
        window->is_open = false;
        window->should_close = true;
    } break;
    case UI_EVENT_WINDOW_RESIZE: {
        window->width = event->resize.width;
        window->height = event->resize.height;
        window->is_dirty = true;
        window->needs_full_redraw = true;
        window->full_redraw_buffers = WINDOW_BUFFER_COUNT;
    } break;
    case UI_EVENT_WINDOW_REDRAW: {
        window->is_dirty = true;
    } break;
    case UI_EVENT_NONE:
    case UI_EVENT_MOUSE_MOVE:
    case UI_EVENT_MOUSE_BUTTON:
    case UI_EVENT_MOUSE_SCROLL:
    case UI_EVENT_KEY:
        break;
    }
}

#if LINUX
#include "linux/wayland/linux_wayland.c"
#elif WINDOWS
#include "win32/window/win32_window.c"
#endif
