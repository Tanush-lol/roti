#ifndef STATE_H
#define STATE_H

#include "w11-client.h"
#include "xdg-shell-client-protocol.h"
#include "scanner.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct frame_slot {
    struct wl_buffer *buffer;
    bool busy;
};

struct frame_buffers {
    uint8_t *data;
    size_t   map_size;
    int      width, height, stride;
    struct frame_slot slot[2];
};

struct our_state {
    struct wl_display    *wl_display;
    struct wl_registry   *wl_registry;
    struct wl_shm        *shm;
    struct wl_compositor *compositor;
    struct xdg_wm_base   *wm_base;
    struct wl_surface    *wl_surface;
    struct xdg_surface   *xdg_surface;
    struct xdg_toplevel  *xdg_toplevel;
    struct frame_buffers fb;
    application *apps;
};

#endif /* STATE_H */
