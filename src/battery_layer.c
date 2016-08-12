#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "constants.h"
#include "settings.h"
#include "radial_layer.h"
#include "battery_layer.h"

typedef struct {
    RadialLayer *radial_layer;
    EventHandle battery_state_event_handle;
    EventHandle settings_event_handle;
} Data;

static void battery_state_handler(BatteryChargeState state, void *context) {
    log_func();
    BatteryLayer *this = (BatteryLayer *) context;
    Data *data = (Data *) layer_get_data(this);
    uint32_t value = state.charge_percent == 0 ? 5 : state.charge_percent == 100 ? 0 : state.charge_percent;
#ifdef PBL_ROUND
    value = value == 0 ? TRIG_MAX_ANGLE : value * (TRIG_MAX_ANGLE / 100);
#endif
    logd("value = %ld", value);
    radial_layer_set_value(data->radial_layer, value);
}

static void settings_handler(Settings *settings, void *context) {
    log_func();
    BatteryLayer *this = (BatteryLayer *) context;
    Data *data = (Data *) layer_get_data(this);
    radial_layer_set_color(data->radial_layer, settings->color_battery);
}

BatteryLayer *battery_layer_create(GRect frame) {
    log_func();
    BatteryLayer *this = layer_create_with_data(frame, sizeof(Data));
    GRect bounds = layer_get_bounds(this);

    Data *data = (Data *) layer_get_data(this);
    data->radial_layer = radial_layer_create(bounds);
    radial_layer_set_thickness(data->radial_layer, RADIAL_THICKNESS);
    layer_add_child(this, data->radial_layer);

    BatteryChargeState state = battery_state_service_peek();
    battery_state_handler(state, this);
    data->battery_state_event_handle = events_battery_state_service_subscribe_context(battery_state_handler, this);

    settings_handler(settings_peek(), this);
    data->settings_event_handle = events_settings_subscribe(settings_handler, this);

    return this;
}

void battery_layer_destroy(BatteryLayer *this) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    events_settings_unsubscribe(data->settings_event_handle);
    events_battery_state_service_unsubscribe(data->battery_state_event_handle);
    radial_layer_destroy(data->radial_layer);
    layer_destroy(this);
}
