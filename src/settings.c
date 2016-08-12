#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>
#include "logging.h"
#include "settings.h"

static const uint32_t SETTINGS_VERSION = 1;
static const uint32_t PERSIST_KEY_VERSION = 0;
static const uint32_t PERSIST_KEY_DATA = 1;

static LinkedRoot *s_handler_list;
static Settings *s_settings;

static AppSync s_sync;
static uint8_t s_sync_buffer[256];

typedef struct {
    EventSettingsHandler handler;
    void *context;
} SettingsHandlerState;

static bool each_settings_saved(void *this, void *context) {
    log_func();
    SettingsHandlerState *state = (SettingsHandlerState *) this;
    Settings *settings = (Settings *) context;
    state->handler(settings, state->context);
    return true;
}

static void save(Settings *this) {
    log_func();
    persist_write_data(PERSIST_KEY_DATA, this, sizeof(Settings));
    persist_write_int(PERSIST_KEY_VERSION, SETTINGS_VERSION);
    if (s_handler_list) linked_list_foreach(s_handler_list, each_settings_saved, this);
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
    log_func();
    loge("sync error %d %d", dict_error, app_message_error);
}

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
    log_func();
    if (new_tuple == NULL || old_tuple == NULL) return;

    Settings *settings = (Settings *) context;
    if (key == MESSAGE_KEY_COLOR_BACKGROUND) {
        settings->color_background = GColorFromHEX(new_tuple->value->uint32);
    } else if (key == MESSAGE_KEY_COLOR_MINUTE) {
        settings->color_minute = GColorFromHEX(new_tuple->value->uint32);
    } else if (key == MESSAGE_KEY_COLOR_HOUR) {
        settings->color_hour = GColorFromHEX(new_tuple->value->uint32);
    } else if (key == MESSAGE_KEY_COLOR_DAY) {
        settings->color_day = GColorFromHEX(new_tuple->value->uint32);
    } else if (key == MESSAGE_KEY_COLOR_MONTH) {
        settings->color_month = GColorFromHEX(new_tuple->value->uint32);
#ifdef PBL_HEALTH
    } else if (key == MESSAGE_KEY_COLOR_HEALTH) {
        settings->color_health = GColorFromHEX(new_tuple->value->uint32);
#endif
    } else if (key == MESSAGE_KEY_COLOR_BATTERY) {
        settings->color_battery = GColorFromHEX(new_tuple->value->uint32);
    } else if (key == MESSAGE_KEY_COLOR_CONNECTION) {
        settings->color_connection = GColorFromHEX(new_tuple->value->uint32);
    } else if (key == MESSAGE_KEY_CONNECTION_VIBE) {
        settings->connection_vibe = atoi(new_tuple->value->cstring);
    }
    save(settings);
}

void settings_init(void) {
    log_func();
    s_settings = malloc(sizeof(Settings));
    int32_t version = persist_read_int(PERSIST_KEY_VERSION);

    if (version == 0) {
        s_settings->color_background = PBL_IF_COLOR_ELSE(GColorLightGray, GColorBlack);
        s_settings->color_minute = PBL_IF_COLOR_ELSE(GColorBlue, GColorWhite);
        s_settings->color_hour = PBL_IF_COLOR_ELSE(GColorVividViolet, GColorLightGray);
        s_settings->color_day = PBL_IF_COLOR_ELSE(GColorTiffanyBlue, GColorWhite);
        s_settings->color_month = PBL_IF_COLOR_ELSE(GColorJazzberryJam, GColorLightGray);
#ifdef PBL_HEALTH
        s_settings->color_health = PBL_IF_COLOR_ELSE(GColorOrange, GColorWhite);
#endif
        s_settings->color_battery = PBL_IF_COLOR_ELSE(GColorIslamicGreen, GColorWhite);
        s_settings->color_connection = PBL_IF_COLOR_ELSE(GColorRed, GColorLightGray);
        s_settings->connection_vibe = ConnectionVibeDisconnect;
        version = 1;
    } else {
        persist_read_data(PERSIST_KEY_DATA, s_settings, sizeof(Settings));
    }

    save(s_settings);

    app_message_open(256, 0);

    Tuplet initial_values[] = {
        TupletInteger(MESSAGE_KEY_COLOR_BACKGROUND, s_settings->color_background.argb),
        TupletInteger(MESSAGE_KEY_COLOR_MINUTE, s_settings->color_minute.argb),
        TupletInteger(MESSAGE_KEY_COLOR_HOUR, s_settings->color_hour.argb),
        TupletInteger(MESSAGE_KEY_COLOR_DAY, s_settings->color_day.argb),
        TupletInteger(MESSAGE_KEY_COLOR_MONTH, s_settings->color_month.argb),
#ifdef PBL_HEALTH
        TupletInteger(MESSAGE_KEY_COLOR_HEALTH, s_settings->color_health.argb),
#endif
        TupletInteger(MESSAGE_KEY_COLOR_BATTERY, s_settings->color_battery.argb),
        TupletInteger(MESSAGE_KEY_COLOR_CONNECTION, s_settings->color_connection.argb),
        TupletInteger(MESSAGE_KEY_CONNECTION_VIBE, s_settings->connection_vibe),
    };
    app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
        sync_changed_handler, sync_error_handler, s_settings);
}

void settings_deinit(void) {
    log_func();
    app_sync_deinit(&s_sync);
    free(s_settings);
}

Settings *settings_peek(void) {
    log_func();
    return s_settings;
}

EventHandle events_settings_subscribe(EventSettingsHandler handler, void *context) {
    log_func();
    if (!s_handler_list) {
        s_handler_list = linked_list_create_root();
    }

    SettingsHandlerState *this = malloc(sizeof(SettingsHandlerState));
    this->handler = handler;
    this->context = context;
    linked_list_append(s_handler_list, this);

    return this;
}

void events_settings_unsubscribe(EventHandle handle) {
    log_func();
    int16_t index = linked_list_find(s_handler_list, handle);
    if (index == -1) {
        return;
    }

    free(linked_list_get(s_handler_list, index));
    linked_list_remove(s_handler_list, index);
    if (linked_list_count(s_handler_list) == 0) {
        free(s_handler_list);
        s_handler_list = NULL;
    }
}
