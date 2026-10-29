#define _POSIX_C_SOURCE 200112L
#include "xdg-shell-client-protocol.h"
#include "w11-client.h"
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include "state.h"
#include "cairo.h"
#include "shm.h"

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    /* Sent by the compositor when it's no longer using this buffer */
    (void)data;
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

struct wl_buffer *draw_frame(struct our_state *state)
{
    const int width  = 1920, height = 1080;
    const int stride = width * 4;
    int size         = stride * height;

    const int shm_pool_size = height * stride * 2;

    int fd = allocate_shm_file(shm_pool_size);
    if (fd == -1) {
        fprintf(stderr, "err in allocate_shm_file()\n");
        return NULL;
    }

    uint8_t *pool_data = (uint8_t *)mmap(NULL, shm_pool_size,
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd, 0);
    if (pool_data == MAP_FAILED) {
        fprintf(stderr, "mmap() failed\n");
        close(fd);
        return NULL;
    }

    int index  = 0;
    int offset = height * stride * index;

    struct wl_shm_pool *pool   = wl_shm_create_pool(state->shm, fd, shm_pool_size);
    struct wl_buffer   *buffer = wl_shm_pool_create_buffer(pool, offset,
                                                            width, height,
                                                            stride,
                                                            WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    drawCairoSquare(pool_data, width, height, offset, stride);
    munmap(pool_data, size);

    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
    return buffer;
}
