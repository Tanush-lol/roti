#define _POSIX_C_SOURCE 200112L
#include "xdg-shell-client-protocol.h"
#include "w11-client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "state.h"
#include "shm.h"
#include "buffer.h"
#include "xdg_surface.h"

static void handle_reg_global(void *data, struct wl_registry *registry,
                               uint32_t name, const char *interface, uint32_t version)
{
    struct our_state *state = (struct our_state *)data;

    if (strcmp(interface, wl_shm_interface.name) == 0) {
        printf("interface : %s | version : %u | name : %u\n", interface, version, name);
        state->shm = (struct wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        printf("interface : %s | version : %u | name : %u\n", interface, version, name);
        state->compositor = (struct wl_compositor *)wl_registry_bind(registry, name,
                                                                       &wl_compositor_interface, 4);
    }

    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        printf("Binding to wm_base\n");
        /* clamp to what the compositor advertises */
        uint32_t bind_version = version < (uint32_t)xdg_wm_base_interface.version
                                    ? version
                                    : (uint32_t)xdg_wm_base_interface.version;
        (void)bind_version; /* used only for documentation; bind at version 1 */
        state->wm_base = (struct xdg_wm_base *)wl_registry_bind(registry, name,
                                                                  &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->wm_base, &xdg_wm_base_listener, state);
    }
} /* handles flushing of available interfaces which we can bind to */

static void handle_reg_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global        = handle_reg_global,
    .global_remove = handle_reg_global_remove,
};

int main(void)
{
    struct our_state state = { 0 };

    const char *display_name    = getenv("WAYLAND_DISPLAY");
    state.wl_display            = wl_display_connect(display_name);
    struct wl_registry *registry = wl_display_get_registry(state.wl_display);

    wl_registry_add_listener(registry, &registry_listener, &state);
    wl_display_roundtrip(state.wl_display);

    state.wl_surface  = wl_compositor_create_surface(state.compositor);
    state.xdg_surface = xdg_wm_base_get_xdg_surface(state.wm_base, state.wl_surface);

    xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);
    state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
    xdg_toplevel_set_title(state.xdg_toplevel, "Example client");
    wl_surface_commit(state.wl_surface);

    while (wl_display_dispatch(state.wl_display)) {
        /* This space deliberately left blank */
    }

    /* wl_registry_destroy(registry); */
    /* wl_display_disconnect(display); */
    return 0;
}
