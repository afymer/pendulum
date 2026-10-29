#pragma once

#include "base/base_inc.h"

#if LINUX
#include "draw/draw_core.h"
#include "window/window_core.h"
#include "window/window_event.h"

#define LINUX_WAYLAND_BUFFER_COUNT 2
#define LINUX_WAYLAND_EVENT_QUEUE_CAPACITY 64
#define LINUX_WAYLAND_MAX_OUTPUTS 8

typedef struct linux_wayland_event_queue_s linux_wayland_event_queue_t;
struct linux_wayland_event_queue_s {
    ui_event_t events[LINUX_WAYLAND_EVENT_QUEUE_CAPACITY];
    u32 head;
    u32 tail;
    u32 count;
};

typedef struct linux_wayland_buffer_s linux_wayland_buffer_t;
struct linux_wayland_buffer_s {
    struct wl_shm_pool *pool;
    struct wl_buffer *buffer;
    u32 *pixels;
    framebuffer_t framebuffer;
    usize capacity;
    bool busy;
    bool needs_full_redraw;
};

typedef struct linux_wayland_window_s linux_wayland_window_t;
struct linux_wayland_window_s {
    window_t base;
    arena_t *arena;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct xdg_wm_base *wm_base;
    struct wl_seat *seat;
    struct wl_keyboard *keyboard;
    u32 modifiers;
    struct wl_output *outputs[LINUX_WAYLAND_MAX_OUTPUTS];
    u32 output_count;
    i32 max_monitor_width;
    i32 max_monitor_height;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    linux_wayland_buffer_t buffers[LINUX_WAYLAND_BUFFER_COUNT];
    struct wl_callback *frame_callback;
    linux_wayland_buffer_t *current;
    linux_wayland_event_queue_t event_queue;
    bool configured;
    bool has_damage;
};
#endif
