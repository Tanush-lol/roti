#ifndef STATE_H
#define STATE_H

/* Pull in the Wayland and xdg-shell types used by the struct below */
#include "w11-client.h"
#include "xdg-shell-client-protocol.h"

struct our_state {
    struct wl_display    *wl_display;
    struct wl_registry   *wl_registry;
    struct wl_shm        *shm;
    struct wl_compositor *compositor;
    struct xdg_wm_base   *wm_base;
    /* Objects */
    struct wl_surface    *wl_surface;
    struct xdg_surface   *xdg_surface;
    struct xdg_toplevel  *xdg_toplevel;
};

#endif /* STATE_H */
