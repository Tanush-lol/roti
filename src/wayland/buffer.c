#define _POSIX_C_SOURCE 200112L
#include "xdg-shell-client-protocol.h"
#include "w11-client.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include "state.h"
#include "cairo_renderer.h"
#include "shm.h"

#define FRAME_WIDTH  1920
#define FRAME_HEIGHT 1080

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    /* compositor is done reading this buffer: it may be drawn into again */
    (void)wl_buffer;
    struct frame_slot *slot = (struct frame_slot *)data;
    slot->busy = false;
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

/* Call once at startup, after state->shm has been bound. Returns 0 on success. */
int init_buffers(struct our_state *state)
{
    struct frame_buffers *fb = &state->fb;

    fb->width    = FRAME_WIDTH;
    fb->height   = FRAME_HEIGHT;
    fb->stride   = fb->width * 4;
    fb->map_size = (size_t)fb->stride * fb->height * 2;   /* two frames */

    int fd = allocate_shm_file(fb->map_size);
    if (fd == -1) {
        fprintf(stderr, "err in allocate_shm_file()\n");
        return -1;
    }

    fb->data = (uint8_t *)mmap(NULL, fb->map_size, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);
    if (fb->data == MAP_FAILED) {
        fprintf(stderr, "mmap() failed\n");
        fb->data = NULL;
        close(fd);
        return -1;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd, (int32_t)fb->map_size);

    for (int i = 0; i < 2; i++) {
        int32_t offset = (int32_t)(fb->stride * fb->height * i);

        fb->slot[i].buffer = wl_shm_pool_create_buffer(pool, offset,
                                                       fb->width, fb->height,
                                                       fb->stride,
                                                       WL_SHM_FORMAT_XRGB8888);
        fb->slot[i].busy = false;
        wl_buffer_add_listener(fb->slot[i].buffer, &wl_buffer_listener, &fb->slot[i]);
    }

    /* buffers keep the memory alive; the pool object and fd are no longer needed */
    wl_shm_pool_destroy(pool);
    close(fd);
    return 0;
}

/* Draws into a free buffer and returns it, or NULL if both are still held by the compositor. */
struct wl_buffer *draw_frame(struct our_state *state)
{
    struct frame_buffers *fb = &state->fb;

    if (fb->data == NULL)
        return NULL;

    for (int i = 0; i < 2; i++) {
        if (fb->slot[i].busy)
            continue;

        int offset = fb->stride * fb->height * i;

        drawCairoSquare(fb->data, fb->width, fb->height, offset, fb->stride);

        fb->slot[i].busy = true;   /* cleared by wl_buffer.release */
        return fb->slot[i].buffer;
    }

    return NULL;
}

void destroy_buffers(struct our_state *state)
{
    struct frame_buffers *fb = &state->fb;

    for (int i = 0; i < 2; i++) {
        if (fb->slot[i].buffer)
            wl_buffer_destroy(fb->slot[i].buffer);
        fb->slot[i].buffer = NULL;
    }
    if (fb->data)
        munmap(fb->data, fb->map_size);
    fb->data = NULL;
}
