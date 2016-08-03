#include <pebble.h>
#include "logging.h"
#include "radial_layer.h"

typedef struct {
    uint16_t thickness;
    uint32_t value;
    GColor color;
} Data;

#ifdef PBL_RECT
typedef struct {
    uint16_t tr;
    uint16_t r;
    uint16_t b;
    uint16_t l;
    uint16_t tl;
} Border;

static Border calculate_border(GRect bounds, uint32_t value) {
    log_func();
    Border border = { 0, 0, 0, 0, 0 };
    uint16_t w = bounds.size.w;
    uint16_t h = bounds.size.h;

    uint32_t pixels = (2 * (w - 1)) + (2 * (h - 1));
    uint32_t progress = (value * pixels) / 100;

    if (progress == 0) {
        border.tr = w / 2;
        border.r = h;
        border.b = w;
        border.l = h;
        border.tl = w / 2;
        return border;
    }

    if (progress < (w / 2)) {
        border.tr = progress;
        return border;
    } else {
        border.tr = w / 2;
    }

    if (progress < ((w / 2) + h)) {
        border.r = progress - (w / 2);
        return border;
    } else {
        border.r = h;
    }

    if (progress < ((uint32_t) ((w / 2) + h + w))) {
        border.b = progress - (w / 2) - h;
        return border;
    } else {
        border.b = w;
    }

    if (progress < ((uint32_t) ((w / 2) + (2 * h) + w))) {
        border.l = progress - (w / 2) - h - w;
        return border;
    } else {
        border.l = h;
        border.tl = progress - (w / 2) - (2 * h) - w;
    }
    return border;
}

static void draw_border(GContext *ctx, GRect bounds, Border border, uint16_t thickness) {
    log_func();
    GRect rect = GRect(bounds.origin.x, bounds.origin.y, border.tl, thickness);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);

    rect = GRect(bounds.origin.x + (bounds.size.w / 2), bounds.origin.y, border.tr, thickness);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);

    rect = GRect(bounds.origin.x, bounds.origin.y + (bounds.size.h - border.l), thickness, border.l);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);

    rect = GRect(bounds.size.w - thickness, bounds.origin.y, thickness, border.r);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);

    rect = GRect(bounds.origin.x + (bounds.size.w - border.b), bounds.size.h - thickness, border.b, thickness);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);
}
#endif

static void update_proc(Layer *this, GContext *ctx) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    GRect bounds = layer_get_bounds(this);
    graphics_context_set_fill_color(ctx, data->color);
#ifdef PBL_ROUND
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, data->thickness, 0, data->value);
#else
    Border border = calculate_border(bounds, data->value);
    draw_border(ctx, bounds, border, data->thickness);
#endif
}

RadialLayer *radial_layer_create(GRect frame) {
    log_func();
    RadialLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, update_proc);
    return this;
}

void radial_layer_destroy(RadialLayer *this) {
    log_func();
    layer_destroy(this);
}

void radial_layer_set_thickness(RadialLayer *this, uint16_t thickness) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    data->thickness = thickness;
    layer_mark_dirty(this);
}

void radial_layer_set_value(RadialLayer *this, uint32_t value) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    data->value = value;
    layer_mark_dirty(this);
}

void radial_layer_set_color(RadialLayer *this, GColor color) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    data->color = color;
    layer_mark_dirty(this);
}
