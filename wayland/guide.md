# Learning wayland from 
http://youtube.com/watch?v=iIVIu7YRdY0&t=735s

# part 1 
<wayland-client.h>
we use this library to communicate with the wayland compositor 
has opaque types,interface objects etc which we would wnna use 

# part 2 
we call wl_display_get_registry() which returns a wl_registry struct 
what is registry and why do we need it?
every program does not pop up directly example when discord might not directly start it might first do auth verification,request the server and then render specific parts of the information as ui
so this registry is not an object think of it like 
struct FILE *fp;
this is just a protocol to connect to the hyprland

compositor is the program which controls your gui example hyprland,Kwin for kde 
compositor is responsible for where each window pops out how every window moves and then binds every window into an image which is displayed on the machine, now this compositor is a game engine basically and is also written using famous game engine libraries like vulkan, openGL

- wl_registry_add_listener() 
this function returns 
