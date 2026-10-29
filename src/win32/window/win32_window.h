#pragma once

#include "base/base_inc.h"
#include "draw/draw_core.h"
#include "window/window_core.h"
#include "window/window_event.h"

#define WIN32_EVENT_QUEUE_CAPACITY 64

typedef struct win32_event_queue_s win32_event_queue_t;
struct win32_event_queue_s {
    ui_event_t events[WIN32_EVENT_QUEUE_CAPACITY];
    u32 head;
    u32 tail;
    u32 count;
};

typedef struct win32_window_s win32_window_t;
struct win32_window_s {
    window_t base;
    arena_t *arena;
    HWND hwnd;
    HANDLE thread;
    DWORD thread_id;
    CRITICAL_SECTION event_lock;
    HANDLE event_signal;
    u32 *pixels;
    usize pixels_capacity;
    framebuffer_t framebuffer;
    win32_event_queue_t event_queue;
};
