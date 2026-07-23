# Learning wayland from 
http://youtube.com/watch?v=iIVIu7YRdY0&t=735s

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

# part 3
we pass in fields `(void *data, struct wl_registry *registry,uint32_t name, const char *interface,uint32_t version)` into the global function now these parameters are sent from the compositor and hence used to bind 
*data* - &state 
*name* - compositor's ID for that object/struct
*interface* - wl_compositor or wl_shm
*version* - highest protocol version supported for the compositor

# part 4 
inside the global function we loop over interfaces as supplied by the compositor and when `strcmp(interface, wl_compositor_interface.name) == 0` i.e interface and wl_compositor_interface.name are same we bind 

### what do we bind ?
