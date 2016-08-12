#ifdef PBL_HEALTH
#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "constants.h"
#include "settings.h"
#include "radial_layer.h"
#include "health_layer.h"

typedef struct {
    RadialLayer *radial_layer;
    EventHandle health_service_event_handle;
    EventHandle settings_event_handle;
    HealthValue goal;
} Data;

static void health_event_handler(HealthEventType event, void *context) {
    log_func();
    HealthLayer *this = (HealthLayer *) context;
    Data *data = layer_get_data(this);

    if (event == HealthEventSignificantUpdate) {
        time_t start = time_start_of_today();
        time_t end = start + SECONDS_PER_DAY;
        HealthServiceAccessibilityMask mask = health_service_metric_averaged_accessible(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
        if (mask & HealthServiceAccessibilityMaskAvailable) {
            data->goal = health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
            health_event_handler(HealthEventMovementUpdate, this);
            layer_set_hidden(this, false);
        } else {
            layer_set_hidden(this, true);
        }
    } else if (event == HealthEventMovementUpdate) {
        HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, time_start_of_today(), time(NULL));
        if (mask & HealthServiceAccessibilityMaskAvailable) {
            HealthValue sum = health_service_sum_today(HealthMetricStepCount);
#ifdef PBL_ROUND
            uint32_t value = sum >= data->goal ? TRIG_MAX_ANGLE : sum * (TRIG_MAX_ANGLE / data->goal);
#else
            uint32_t value = (sum * 100) / data->goal;
#endif
            logd("value = %ld", value);
            radial_layer_set_value(data->radial_layer, value);
            layer_set_hidden(this, false);
        } else {
            layer_set_hidden(this, true);
        }
    }
}

static void settings_handler(Settings *settings, void *context) {
    log_func();
    HealthLayer *this = (HealthLayer *) context;
    Data *data = layer_get_data(this);
    radial_layer_set_color(data->radial_layer, settings->color_health);
}

HealthLayer *health_layer_create(GRect frame) {
    log_func();
    HealthLayer *this = layer_create_with_data(frame, sizeof(Data));
    GRect bounds = layer_get_bounds(this);

    Data *data = (Data *) layer_get_data(this);
    data->radial_layer = radial_layer_create(bounds);
    radial_layer_set_thickness(data->radial_layer, RADIAL_THICKNESS);
    layer_add_child(this, data->radial_layer);

    health_event_handler(HealthEventSignificantUpdate, this);
    data->health_service_event_handle = events_health_service_events_subscribe(health_event_handler, this);

    settings_handler(settings_peek(), this);
    data->settings_event_handle = events_settings_subscribe(settings_handler, this);

    return this;
}

void health_layer_destroy(HealthLayer *this) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    events_settings_unsubscribe(data->settings_event_handle);
    events_health_service_events_unsubscribe(data->health_service_event_handle);
    radial_layer_destroy(data->radial_layer);
    layer_destroy(this);
}
#endif // PBL_HEALTH
