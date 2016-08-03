#include <pebble.h>
#include "logging.h"
#include "constants.h"
#include "minute_layer.h"
#include "hour_layer.h"
#include "battery_layer.h"
#ifdef PBL_HEALTH
#include "health_layer.h"

static HealthLayer *s_health_layer;
#endif
static Window *s_window;
static MinuteLayer *s_minute_layer;
static HourLayer *s_hour_layer;
static BatteryLayer *s_battery_layer;

static void window_load(Window *window) {
    log_func();
    window_set_background_color(window, PBL_IF_COLOR_ELSE(GColorLightGray, GColorBlack));

    Layer *root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);

    GRect inset = bounds;
    s_minute_layer = minute_layer_create(inset);
    layer_add_child(root_layer, s_minute_layer);

    inset = grect_crop(inset, RADIAL_THICKNESS);
    s_hour_layer = hour_layer_create(inset);
    layer_add_child(root_layer, s_hour_layer);

#ifdef PBL_HEALTH
    inset = grect_crop(inset, RADIAL_THICKNESS);
    s_health_layer = health_layer_create(inset);
    layer_add_child(root_layer, s_health_layer);
#endif

    inset = grect_crop(inset, RADIAL_THICKNESS);
    s_battery_layer = battery_layer_create(inset);
    layer_add_child(root_layer, s_battery_layer);
}

static void window_unload(Window *window) {
    log_func();
    battery_layer_destroy(s_battery_layer);
#ifdef PBL_HEALTH
    health_layer_destroy(s_health_layer);
#endif
    hour_layer_destroy(s_hour_layer);
    minute_layer_destroy(s_minute_layer);
}

static void init(void) {
    log_func();
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });
    window_stack_push(s_window, true);
}

static void deinit(void) {
    log_func();
    window_destroy(s_window);
}

int main(void) {
    log_func();
    init();
    app_event_loop();
    deinit();
}
