#include <stdint.h>
#include <stdio.h>
#include <wayland-client.h>
#include <string.h>
struct our_state{
  struct wl_compositor  *compositor;
  struct wl_shm *shm;
  struct wl_surface *surface;
  struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, offset,width, height, stride, WL_SHM_FORMAT_XRGB8888);
};
static void registry_handle_global(void *data, struct wl_registry *registry,uint32_t name, const char *interface,uint32_t version)
{
    struct our_state *state = data ;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
       printf("interface:     %s\ncomp_interface:%s \n",interface,wl_compositor_interface.name);
       printf("interface and wl_compositor_interface are equal\n");    
        state->compositor = wl_registry_bind(
            registry, name, &wl_compositor_interface,1);
    }

    if (strcmp(interface, wl_shm_interface.name) == 0 ) {
     printf("interface:     %s\nshm_interface:%s \n",interface,wl_shm_interface.name);
     printf("interface and wl_shm_interface.name are equal\n");
      state->shm = wl_registry_bind(registry,name,&wl_shm_interface,1); 
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
  struct our_state state = {0};
	wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_roundtrip(display);
  state.surface = wl_compositor_create_surface(state.compositor);
	return 0;
}

