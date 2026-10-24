#ifndef XDG_SURFACE_H
#define XDG_SURFACE_H

#include "w11-client.h"
#include "xdg-shell-client-protocol.h"
#include "state.h"

void xdg_surface_configure(void *pool_data, struct xdg_surface *xdg_surface, uint32_t serial);
void xdg_wm_base_ping(void *pool_data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);

extern const struct xdg_surface_listener xdg_surface_listener;
extern const struct xdg_wm_base_listener xdg_wm_base_listener;

#endif /* XDG_SURFACE_H */
