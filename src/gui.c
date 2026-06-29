#include <wayland-client.h>
#include <stdio.h>


struct wl_display *display;

int main(){
  display =  wl_display_connect(NULL);

  if (!display) {
    printf("Error\n");
    perror("wayland_display");
  }else{
    printf("Successfull");
  }

  return 0;
}
