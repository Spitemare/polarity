#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include "logging.h"
#include "settings.h"
#include "connection.h"

#ifdef PBL_HEALTH
static bool s_sleeping;
static EventHandle s_health_event_handle;
#endif

static bool s_connected;
static EventHandle s_connection_service_event_handle;

#ifdef PBL_HEALTH
static void health_event_handler(HealthEventType event, void *context) {
    log_func();
    if (event == HealthEventSignificantUpdate) {
        health_event_handler(HealthEventSleepUpdate, context);
    } else if (event == HealthEventSleepUpdate) {
        HealthActivityMask mask = health_service_peek_current_activities();
        s_sleeping = (mask & HealthActivitySleep) || (mask & HealthActivityRestfulSleep);
    }
}
#endif

static void connection_handler(bool connected) {
    log_func();
#ifdef PBL_HEALTH
    if (s_sleeping) return;
#endif

    Settings *settings = settings_peek();
    switch (settings->connection_vibe) {
        case ConnectionVibeNone:
            return;
        case ConnectionVibeDisconnect:
            if (!connected) vibes_double_pulse();
            break;
        case ConnectionVibeDisconnectAndReconnect:
            if (s_connected != connected) vibes_double_pulse();
            break;
    }
    s_connected = connected;
}

void connection_init(void) {
    log_func();
#ifdef PBL_HEALTH
    s_health_event_handle = events_health_service_events_subscribe(health_event_handler, NULL);
#endif

    s_connected = connection_service_peek_pebble_app_connection();
    s_connection_service_event_handle = events_connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = connection_handler
    });
}

void connection_deinit(void) {
    log_func();
    events_connection_service_unsubscribe(s_connection_service_event_handle);
#ifdef PBL_HEALTH
    events_health_service_events_unsubscribe(s_health_event_handle);
#endif
}
