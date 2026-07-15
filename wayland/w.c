#include <wayland-client.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void reg_glob(void *data,
              struct wl_registry *reg,
              uint32_t name,
              const char *interface,
              uint32_t version)
{
    printf("Name: %u Interface: %s Version: %u\n",
           name,
           interface,
           version);
}

void reg_glob_rem(void *data,struct wl_registry *reg,uint32_t name){

}

struct wl_registry_listener reg_list = {
  .global = reg_glob,
  .global_remove = reg_glob_rem,
};

int main(){
  struct wl_display *disp = wl_display_connect(0);
  if (!disp) {
    return -1;
  }
  struct wl_registry *reg = wl_display_get_registry(disp);
  wl_registry_add_listener(reg, &reg_list,0);
  wl_display_roundtrip(disp);

  wl_display_disconnect(disp);
  return 0;
}
