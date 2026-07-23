a project to replace roofi, cuh i think i can do better 

NOTES:
getenv("variable")

this is the function we use to look directly into host operating system's process memory and get the environment variable

how to use the nix-shell file 
install nix acc to ur os 
and then cd into this project and type "nix-shell"

# ***Learning Wayland***

# part 1 
<wayland-client.h>
we use this library to communicate with the wayland compositor 
has opaque types,interface objects etc which we would wnna use 

# part 2 
## wl_display_get_registry() 
which returns a wl_registry struct 
 - what is registry and why do we need it?
so the registry is the object that will give u the struct to communicate with the compositor which is standardized with elements 
1 -> wl_compositor
2 -> wl_shm
3 -> wl_seat
4 -> xdg_wm_base
but the numbers refrencing them might differ so we need to call this function to communicate with the server
**compositor** is the program which controls your gui example hyprland,Kwin for kde 

## wl_registry_add_listener() 
this function adds a listener to our registry we pass on the `struct wl_registry_listener registry_listener` address into the function which is a struct with functions *registry_handle_global* *registry_handle_global_remove* which are the functions to handle the registry and we handle the functions composed into a struct to maintain reproducability with new versions of wayland,supposedly a design pattern u can keep in mind 
and last on we pass state, state is the struct where we use the fields of the struct registry, 
if your look closely **state** the fields are 
  struct wl_compositor  *compositor;
  struct wl_shm *shm;
 and more
and so we are using the fields to communicate with the server to render a window
we pass state first time to let the server know what fields of registry are we gonna work with and 2nd time in global function to actually use those functions

# part 3
we pass in fields `(void *data, struct wl_registry *registry,uint32_t name, const char *interface,uint32_t version)` into the global function now these parameters are sent from the compositor and hence used to bind 
*data* - &state 
*name* - compositor's ID for that object/struct
*interface* - wl_compositor or wl_shm
*version* - highest protocol version supported for the compositor

# part 4 
inside the global function we loop over interfaces as supplied by the compositor and when `strcmp(interface, wl_compositor_interface.name) == 0` i.e interface and wl_compositor_interface.name are same we bind 

### what do we bind ?
we use the function wl_registry_bind function to bind data from the compositor to the struct our_state and the integer we passed on is just version protocol

next we use wl_surface which is an object used to create a surface this won't be visible btw 

# part 5 

now we need to use XDG shell for rendering a surface
