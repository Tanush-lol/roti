#include <stdint.h>
#include <stdio.h>
#include <wayland-client.h>
#include <string.h>

struct our_state{
  struct wl_compositor  *compositor;
};

    struct our_state state ;
static void registry_handle_global(void *data, struct wl_registry *registry,uint32_t name, const char *interface,uint32_t version)
{
    state.compositor = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state.compositor = wl_registry_bind(
            registry, name, &wl_compositor_interface, 4);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

int
main(int argc, char *argv[])
{
	struct wl_display *display = wl_display_connect(0);
  if (!display) {
    return -1;
  }
	struct wl_registry *registry = wl_display_get_registry(display);
  
	wl_registry_add_listener(registry, &registry_listener, &state);

	wl_display_roundtrip(display);
	return 0;
}
