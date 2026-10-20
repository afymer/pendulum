#pragma once

#include "window/window_inc.h"

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
    struct wl_buffer *buffer;
    framebuffer_t framebuffer; // mapping of the buffer, fixed at the first configure size
    usize framebuffer_size;    // in bytes
    bool configured;
};
