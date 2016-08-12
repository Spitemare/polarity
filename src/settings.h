#pragma once

typedef enum {
    ConnectionVibeNone = 0,
    ConnectionVibeDisconnect,
    ConnectionVibeDisconnectAndReconnect
} ConnectionVibe;

typedef struct {
    GColor color_background;
    GColor color_minute;
    GColor color_hour;
#ifdef PBL_HEALTH
    GColor color_health;
#endif
    GColor color_battery;
    GColor color_connection;
    ConnectionVibe connection_vibe;
} Settings;

void settings_init(void);
void settings_deinit(void);
Settings *settings_peek(void);

typedef void* EventHandle;
typedef void(*EventSettingsHandler)(Settings *settings, void *context);

EventHandle events_settings_subscribe(EventSettingsHandler handler, void *context);
void events_settings_unsubscribe(EventHandle handle);
