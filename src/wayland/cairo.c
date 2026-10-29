#define _GNU_SOURCE   /* exposes M_PI in math.h */
#include <cairo/cairo.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* top left corner's coordinates are 0,0 */

/* drawArc and drawRoundedSquare are helpers kept for reference;
 * suppress -Wunused-function by marking them with __attribute__((unused)). */
static void drawArc(cairo_t *cr) __attribute__((unused));
static void drawArc(cairo_t *cr)
{
    cairo_translate(cr, 0.2, 0.2);
    cairo_scale(cr, 2, 2);
    double xc     = 128.0;
    double yc     = 128.0;
    double radius = 100.0;
    double angle1 = 45.0  * (M_PI / 180.0); /* angles are specified */
    double angle2 = 180.0 * (M_PI / 180.0); /* in radians           */

    cairo_set_line_width(cr, 10.0);
    cairo_arc(cr, xc, yc, radius, angle1, angle2);
    cairo_stroke(cr);

    /* draw helping lines */
    cairo_set_source_rgba(cr, 1, 0.2, 0.2, 0.6);
    cairo_set_line_width(cr, 6.0);

    cairo_arc(cr, xc, yc, 10.0, 0, 2 * M_PI);
    cairo_fill(cr);

    cairo_arc(cr, xc, yc, radius, angle1, angle1);
    cairo_line_to(cr, xc, yc);
    cairo_arc(cr, xc, yc, radius, angle2, angle2);
    cairo_line_to(cr, xc, yc);
    cairo_stroke(cr);
}

static void drawRoundedSquare(cairo_t *cr) __attribute__((unused));
static void drawRoundedSquare(cairo_t *cr)
{
    cairo_scale(cr, 0.5, 0.5);
    cairo_matrix_t mat;
    cairo_matrix_init(&mat, 20, 0, 0, 20, 0, 0);
    cairo_transform(cr, &mat);

    double x0          = 15.6;
    double y0          = 15.6;
    double rect_width  = 20;
    double rect_height = 20;
    double radius      = 10; /* approximate curvature radius */

    double x1, y1;
    x1 = x0 + rect_width;
    y1 = y0 + rect_height;

    if (!rect_width || !rect_height)
        return;

    if (rect_width / 2 < radius) {
        if (rect_height / 2 < radius) {
            cairo_move_to (cr, x0, (y0 + y1) / 2);
            cairo_curve_to(cr, x0, y0, x0, y0, (x0 + x1) / 2, y0);
            cairo_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1) / 2);
            cairo_curve_to(cr, x1, y1, x1, y1, (x1 + x0) / 2, y1);
            cairo_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1) / 2);
        } else {
            cairo_move_to (cr, x0, y0 + radius);
            cairo_curve_to(cr, x0, y0, x0, y0, (x0 + x1) / 2, y0);
            cairo_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
            cairo_line_to (cr, x1, y1 - radius);
            cairo_curve_to(cr, x1, y1, x1, y1, (x1 + x0) / 2, y1);
            cairo_curve_to(cr, x0, y1, x0, y1, x0, y1 - radius);
        }
    } else {
        if (rect_height / 2 < radius) {
            cairo_move_to (cr, x0, (y0 + y1) / 2);
            cairo_curve_to(cr, x0, y0, x0, y0, x0 + radius, y0);
            cairo_line_to (cr, x1 - radius, y0);
            cairo_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1) / 2);
            cairo_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
            cairo_line_to (cr, x0 + radius, y1);
            cairo_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1) / 2);
        } else {
            cairo_move_to (cr, x0, y0 + radius);
            cairo_curve_to(cr, x0, y0, x0, y0, x0 + radius, y0);
            cairo_line_to (cr, x1 - radius, y0);
            cairo_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
            cairo_line_to (cr, x1, y1 - radius);
            cairo_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
            cairo_line_to (cr, x0 + radius, y1);
            cairo_curve_to(cr, x0, y1, x0, y1, x0, y1 - radius);
        }
    }
    cairo_close_path(cr);

    cairo_set_source_rgb (cr, 0.5, 0.5, 1);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 0.5, 0, 0, 0.5);
    cairo_set_line_width(cr, 10.0);
    cairo_stroke(cr);
}

static void renderText(cairo_t *cr,const char *str,int x,int y)
{
    cairo_select_font_face(cr, "Sans",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30);

    /* Text colour: black */
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

    cairo_move_to(cr, x, y);

    char label[64];
    snprintf(label, sizeof(label), "(%d)",str);
    cairo_show_text(cr, label);
}

void draw

void drawCairoSquare(uint8_t *pool_data, int width, int height, int offset, int stride)
{
    uint32_t *pixels = (uint32_t *)&pool_data[offset];

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        (unsigned char *)pixels,
        CAIRO_FORMAT_ARGB32,
        width, height, stride);

    cairo_t *cr = cairo_create(surface);

    /* Draw a background */
    cairo_set_source_rgb(cr, 0.93, 0.93, 0.93);
    cairo_paint(cr);

    /* Draw a black outlined square */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 20, 20, 80, 80);
    cairo_stroke(cr);

    /* drawArc(cr); */
    renderText(cr, 100, 30);
    renderText(cr, 100, 90);
    renderText(cr, 100, 150);

    /* Finish drawing before handing the buffer to Wayland */
    cairo_destroy(cr);
    cairo_surface_flush(surface);
    cairo_surface_destroy(surface);
}
