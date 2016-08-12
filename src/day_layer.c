#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "constants.h"
#include "settings.h"
#include "radial_layer.h"
#include "day_layer.h"

typedef struct {
    RadialLayer *radial_layer;
    EventHandle tick_timer_event_handle;
    EventHandle settings_event_handle;
    int days_in_month;
} Data;

static int get_days_in_month(struct tm* tick_time) {
    log_func();
    int month = tick_time->tm_mon + 1;
    if (month == 2) { // February
        int year = tick_time->tm_year;
        bool leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap_year ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 31;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed, void *context) {
    log_func();
    DayLayer *this = (DayLayer *) context;
    Data *data = (Data *) layer_get_data(this);

    if (units_changed & MONTH_UNIT) {
        data->days_in_month = get_days_in_month(tick_time);
    }

#ifdef PBL_ROUND
    uint32_t value = tick_time->tm_mday * (TRIG_MAX_ANGLE / data->days_in_month);
#else
    uint32_t value = (tick_time->tm_mday * 100) / data->days_in_month;
#endif
    logd("value = %ld", value);
    radial_layer_set_value(data->radial_layer, value);
}

static void settings_handler(Settings *settings, void *context) {
    log_func();
    DayLayer *this = (DayLayer *) context;
    Data *data = (Data *) layer_get_data(this);
    radial_layer_set_color(data->radial_layer, settings->color_day);
}

DayLayer *day_layer_create(GRect frame) {
    log_func();
    DayLayer *this = layer_create_with_data(frame, sizeof(Data));
    GRect bounds = layer_get_bounds(this);

    Data *data = (Data *) layer_get_data(this);
    data->radial_layer = radial_layer_create(bounds);
    radial_layer_set_thickness(data->radial_layer, SMALL_RADIAL_THICKNESS);
    layer_add_child(this, data->radial_layer);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    tick_handler(t, MONTH_UNIT | DAY_UNIT, this);
    data->tick_timer_event_handle = events_tick_timer_service_subscribe_context(DAY_UNIT, tick_handler, this);

    settings_handler(settings_peek(), this);
    data->settings_event_handle = events_settings_subscribe(settings_handler, this);

    return this;
}

void day_layer_destroy(DayLayer *this) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    events_settings_unsubscribe(data->settings_event_handle);
    events_tick_timer_service_unsubscribe(data->tick_timer_event_handle);
    radial_layer_destroy(data->radial_layer);
    layer_destroy(this);
}
