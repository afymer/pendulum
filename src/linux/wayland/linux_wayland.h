#pragma once

#include "window/window_inc.h"

#define LINUX_WAYLAND_BUFFER_COUNT 2

typedef struct linux_wayland_buffer_s linux_wayland_buffer_t;
struct linux_wayland_buffer_s {
    struct wl_buffer *buffer;
    framebuffer_t framebuffer;
    usize size;
    bool busy;
};

typedef struct linux_wayland_window_s linux_wayland_window_t;
struct linux_wayland_window_s {
    window_t base;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct xdg_wm_base *wm_base;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    linux_wayland_buffer_t buffers[LINUX_WAYLAND_BUFFER_COUNT];
    struct wl_callback *frame_callback;
    linux_wayland_buffer_t *current;
    bool configured;
};
