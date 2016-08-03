#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "constants.h"
#include "radial_layer.h"
#include "hour_layer.h"

typedef struct {
    RadialLayer *radial_layer;
    EventHandle tick_timer_event_handle;
} Data;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed, void *context) {
    log_func();
    HourLayer *this = (HourLayer *) context;
    Data *data = (Data *) layer_get_data(this);
    int hour = tick_time->tm_hour % 12;
#ifdef PBL_ROUND
    uint32_t value = hour == 0 ? TRIG_MAX_ANGLE : hour * (TRIG_MAX_ANGLE / 12);
#else
    uint32_t value = (hour * 100) / 12;
#endif
    logd("value = %ld", value);
    radial_layer_set_value(data->radial_layer, value);
}

HourLayer *hour_layer_create(GRect frame) {
    log_func();
    HourLayer *this = layer_create_with_data(frame, sizeof(Data));
    GRect bounds = layer_get_bounds(this);

    Data *data = (Data *) layer_get_data(this);
    data->radial_layer = radial_layer_create(bounds);
    radial_layer_set_thickness(data->radial_layer, RADIAL_THICKNESS);
    radial_layer_set_color(data->radial_layer, PBL_IF_COLOR_ELSE(GColorVividViolet, GColorLightGray));
    layer_add_child(this, data->radial_layer);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    tick_handler(t, HOUR_UNIT, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(HOUR_UNIT, tick_handler, this);

    return this;
}

void hour_layer_destroy(HourLayer *this) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    radial_layer_destroy(data->radial_layer);
    layer_destroy(this);
}
