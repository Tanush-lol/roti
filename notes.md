## **libdrm**
C API for working with these device nodes. One such library is libdrm,

## **Mesa**
vendor-optimized implementations of OpenGL (and Vulkan) for Linux and the GBM (Generic Buffer Management) library — an abstraction on top of libdrm for allocating buffers on the GPU

## **Libinp**
 libinput provides the userspace end of evdev. It's responsible for receiving input events from the kernel from your various input devices, decoding them into a usable form, and passing them on to the Wayland compositor.

## **(e)udev**
device nodes in /dev, and sending word of these changes to applications running on your system,

## **xkbcommon**
XKB, short for X keyboard the original keyboard handling subsystem of the Xorg server

## **pixman**
efficiently manipulating pixel buffers, doing math with intersecting rectangles, and performing other similar pixel manipulation tasks.

## **libwayland**
 low-level wire protocol.

#### ***wayland.xml,wayland-scanner,libwayland***
two libraries, libwayland-client and libwayland-server, include an implementation of the wire protocol

- object is an entity known to both the client and server 
- libwayland refers to these objects through the wl_proxy interface
- On the server, objects are abstracted through wl_resource, which is fairly similar, but have an extra degree of complexity — the server has to track which object belongs to which client. Each wl_resource is owned by a single client.

## **Protocol design patterns**
**Atomicity**
- most interfaces allow you to update them transactionally, using several requests to build up a new representation of its state, then committing them all at once, 
- updated in buffer format
- interface includes separate requests for configuring each of these, but these are applied to a pending state. Only when the commit request is sent does the pending state get merged into the current state,

**Resource lifetimes**
- interfaces which define resources that have finite lifetimes will often include requests and events through which the client or server can state their intention to no longer send requests or events for that object
- once both sides have agreed to this — asynchronously — do they destroy the resources they allocated for that object.
- client is listening for events and sending requests, and a server listens for requests and sends events.


# XDG Protocol files generation history

 g++ -Iinclude src/main.cpp src/shm.cpp -o app -lwayland-client
/usr/bin/ld: /tmp/ccSLJRfD.o: warning: relocation against `xdg_surface_interface' in read-only section `.text'
/usr/bin/ld: /tmp/ccSLJRfD.o: in function `xdg_wm_base_get_xdg_surface':
main.cpp:(.text+0x340): undefined reference to `xdg_surface_interface'
/usr/bin/ld: warning: creating DT_TEXTREL in a PIE
collect2: error: ld returned 1 exit status

 [tanush@Tanush w11practice] 
❯ gcc -Iinclude -c src/xdg-shell-protocol.c \
    -o src/xdg-shell-protocol.o
cc1: fatal error: src/xdg-shell-protocol.c: No such file or directory
compilation terminated.

 [tanush@Tanush w11practice] 
❯ gcc -Iinclude -c include/xdg-shell-protocol.c     -o src/xdg-shell-protocol.o

 [tanush@Tanush w11practice] 
❯ g++ -Iinclude \
    src/main.cpp \
    src/shm.cpp \
    src/xdg-shell-protocol.o \
    -o app \
    -lwayland-client

 [tanush@Tanush w11practice] 
❯
g++ -Iinclude src/main.cpp src/shm.cpp src/xdg-shell-protocol.o -o app -lwayland-client
g++ sideQuests/cairo.cpp -o a $(pkg-config --cflags --libs cairo)

cmake -S . -B build
cmake --build build -j"$(nproc)"
./build/roti
