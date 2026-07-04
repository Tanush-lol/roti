a project to replace roofi, cuh i think i can do better 

NOTES:
getenv("variable")

this is the function we use to look directly into host operating system's process memory and get the environment variable

how to use the nix-shell file 
install nix acc to ur os 
and then cd into this project and type "nix-shell"

learning all about makefiles
makefiles is similar to bash scripting
so compile the gui file with gcc main.c -o main $(pkg-config --cflags --libs wayland-client)

pkg-config is a tool that knows where libraries are installed on your system.
example "pkg-config --cflags wayland-client" on bash will output "-I/usr/include"
and "pkg-config --libs wayland-client" will print "-lwayland-client" on bash
