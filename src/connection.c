#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "connection.h"

static bool s_connected;
static EventHandle s_connection_service_event_handle;

static void connection_handler(bool connected) {
    log_func();
    if (s_connected != connected) vibes_double_pulse();
    s_connected = connected;
}

void connection_init(void) {
    log_func();
    s_connected = connection_service_peek_pebble_app_connection();
    s_connection_service_event_handle = events_connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = connection_handler
    });
}

void connection_deinit(void) {
    log_func();
    events_connection_service_unsubscribe(s_connection_service_event_handle);
}
