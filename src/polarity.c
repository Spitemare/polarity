#include <pebble.h>
#include "logging.h"
#include "constants.h"
#include "settings.h"
#include "connection.h"
#include "minute_layer.h"
#include "hour_layer.h"
#ifdef PBL_HEALTH
#include "health_layer.h"
#endif
#include "battery_layer.h"
#include "connection_layer.h"

static Window *s_window;
static MinuteLayer *s_minute_layer;
static HourLayer *s_hour_layer;
#ifdef PBL_HEALTH
static HealthLayer *s_health_layer;
#endif
static BatteryLayer *s_battery_layer;
static ConnectionLayer *s_connection_layer;

static EventHandle s_settings_event_handle;

static void settings_handler(Settings *settings, void *context) {
    log_func();
    Window *window = (Window *) context;
    window_set_background_color(window, settings->color_background);
}

static void window_load(Window *window) {
    log_func();

    Layer *root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);
    GRect inset = bounds;

    s_minute_layer = minute_layer_create(inset);
    layer_add_child(root_layer, s_minute_layer);
    inset = grect_crop(inset, RADIAL_THICKNESS);

    s_hour_layer = hour_layer_create(inset);
    layer_add_child(root_layer, s_hour_layer);
    inset = grect_crop(inset, RADIAL_THICKNESS);

#ifdef PBL_HEALTH
    s_health_layer = health_layer_create(inset);
    layer_add_child(root_layer, s_health_layer);
    inset = grect_crop(inset, RADIAL_THICKNESS);
#endif

    s_battery_layer = battery_layer_create(inset);
    layer_add_child(root_layer, s_battery_layer);
    inset = grect_crop(inset, RADIAL_THICKNESS);

    s_connection_layer = connection_layer_create(inset);
    layer_add_child(root_layer, s_connection_layer);

    settings_handler(settings_peek(), window);
    s_settings_event_handle = events_settings_subscribe(settings_handler, window);
}

static void window_unload(Window *window) {
    log_func();
    events_settings_unsubscribe(s_settings_event_handle);

    connection_layer_destroy(s_connection_layer);
    battery_layer_destroy(s_battery_layer);
#ifdef PBL_HEALTH
    health_layer_destroy(s_health_layer);
#endif
    hour_layer_destroy(s_hour_layer);
    minute_layer_destroy(s_minute_layer);
}

static void init(void) {
    log_func();
    settings_init();
    connection_init();

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

    connection_deinit();
    settings_deinit();
}

int main(void) {
    log_func();
    init();
    app_event_loop();
    deinit();
}
