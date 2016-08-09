#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "constants.h"
#include "radial_layer.h"
#include "connection_layer.h"

typedef struct {
    EventHandle connection_service_event_handle;
} Data;

static void update_proc(Layer *this, GContext *ctx) {
    log_func();
    GRect bounds = layer_get_bounds(this);
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray));
#ifdef PBL_ROUND
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, RADIAL_THICKNESS, 0, DEG_TO_TRIGANGLE(360));
#else
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
#endif
}

static void connection_handler(bool connected, void *context) {
    log_func();
    ConnectionLayer *this = (ConnectionLayer *) context;
    layer_set_hidden(this, !connected);
    layer_mark_dirty(this);
}

ConnectionLayer *connection_layer_create(GRect frame) {
    log_func();
    ConnectionLayer *this = layer_create_with_data(frame, sizeof(Data));
    layer_set_update_proc(this, update_proc);

    Data *data = layer_get_data(this);
    data->connection_service_event_handle = events_connection_service_subscribe_context((EventConnectionHandlers) {
        .pebble_app_connection_handler = connection_handler
    }, this);

    return this;
}

void connection_layer_destroy(ConnectionLayer *this) {
    log_func();
    Data *data = (Data *) layer_get_data(this);
    events_connection_service_unsubscribe(data->connection_service_event_handle);
    layer_destroy(this);
}
