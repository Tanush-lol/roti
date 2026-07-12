#include <wayland-client.h>
#include <stdio.h>



int main(){

  struct wl_display *display =  wl_display_connect(NULL);

  if (!display) {
    printf("Error\n");
    perror("wayland_display");
  }else{
    printf("Successfull");
  }

  wl_display_disconnect(display);
  return 0;
}
