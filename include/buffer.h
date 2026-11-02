#ifndef BUFFER_H
#define BUFFER_H

#include "w11-client.h"
#include "state.h"

struct wl_buffer *draw_frame(struct our_state *state);
int  init_buffers(struct our_state *state);
void destroy_buffers(struct our_state *state);

#endif /* BUFFER_H */
