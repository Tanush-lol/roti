#define _POSIX_C_SOURCE 200112L
#include "xdg-shell-client-protocol.h"
#include "w11-client.h"
#include "state.h"
#include "buffer.h"

void xdg_surface_configure(void *pool_data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    struct our_state *state = (struct our_state *)pool_data;
    xdg_surface_ack_configure(xdg_surface, serial);

    struct wl_buffer *buffer = draw_frame(state);
    wl_surface_attach(state->wl_surface, buffer, 0, 0);
    /* wl_surface_damage(state->wl_surface, 0, 0, UINT32_MAX, INT32_MAX); */
    wl_surface_commit(state->wl_surface);
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

void xdg_wm_base_ping(void *pool_data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    (void)pool_data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};
