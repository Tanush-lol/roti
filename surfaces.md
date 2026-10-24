## theory
- no state at all when it's first created.
- pending state is stacked and when server thinks event has affected the surface,applied when wl_surface.commit  is sent 
    in main.cpp **wl_surface_commit(state.wl_surface);**
- the surface's role may have additional double-buffered state like this
- To make it valid,provide the necessary information,example giving it a role (like xdg_toplevel), allocating a  nd attaching a buffer 
    in main.cpp **state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);**
- 

## code 

## functions

## constants
