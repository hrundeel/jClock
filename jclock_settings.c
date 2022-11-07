#include <furi.h>
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>
//#include <lib/toolbox/value_index.h>
#include "jclock_settings.h"

#define TAG "jClock"

typedef struct {
    ClockSettings clock_settings;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
} ClockAppSettings;

static uint32_t jclock_settings_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

#define JTIME_FORMAT_COUNT 2
const char* const jtime_format_text[JTIME_FORMAT_COUNT] = {
    "12h",
    "24h",
};

const uint32_t jtime_format_value[JTIME_FORMAT_COUNT] = { H12, H24 };

#define JDATE_FORMAT_COUNT 2
const char* const jdate_format_text[JDATE_FORMAT_COUNT] = {
    "mm-dd", // ISO 8601
    "dd-mm", // RFC 5322
};

const uint32_t jdate_format_value[JDATE_FORMAT_COUNT] = { Iso, Rfc };

//JJY

#define JJY_ENABLED_COUNT 2
const char* const jjy_enabled_text[JJY_ENABLED_COUNT] = {
    "Off",  // JJY enabled auto
    "On",   //     disabled
};

const uint32_t jjy_enabled_value[JJY_ENABLED_COUNT] = { DISABLED, ENABLED };


//#define JJY_DTZ_COUNT 193
const char* const jjy_dtz_text[JJY_DTZ_COUNT] = {
    "-24:00",
    "-23:45", "-23:30", "-23:15", "-23:00",
    "-22:45", "-22:30", "-22:15", "-22:00",
    "-21:45", "-21:30", "-21:15", "-21:00",
    "-20:45", "-20:30", "-20:15", "-20:00",
    "-19:45", "-19:30", "-19:15", "-19:00",
    "-18:45", "-18:30", "-18:15", "-18:00",
    "-17:45", "-17:30", "-17:15", "-17:00",
    "-16:45", "-16:30", "-16:15", "-16:00",
    "-15:45", "-15:30", "-15:15", "-15:00",
    "-14:45", "-14:30", "-14:15", "-14:00",
    "-13:45", "-13:30", "-13:15", "-13:00",
    "-12:45", "-12:30", "-12:15", "-12:00",
    "-11:45", "-11:30", "-11:15", "-11:00",
    "-10:45", "-10:30", "-10:15", "-10:00",
    "-9:45", "-9:30", "-9:15", "-9:00",
    "-8:45", "-8:30", "-8:15", "-8:00",
    "-7:45", "-7:30", "-7:15", "-7:00",
    "-6:45", "-6:30", "-6:15", "-6:00",
    "-5:45", "-5:30", "-5:15", "-5:00",
    "-4:45", "-4:30", "-4:15", "-4:00",
    "-3:45", "-3:30", "-3:15", "-3:00",
    "-2:45", "-2:30", "-2:15", "-2:00",
    "-1:45", "-1:30", "-1:15", "-1:00",
    "-0:45", "-0:30", "-0:15",
    "--0--",
    "+0:15", "+0:30", "+0:45",
    "+1:00", "+1:15", "+1:30", "+1:45",
    "+2:00", "+2:15", "+2:30", "+2:45",
    "+3:00", "+3:15", "+3:30", "+3:45",
    "+4:00", "+4:15", "+4:30", "+4:45",
    "+5:00", "+5:15", "+5:30", "+5:45",
    "+6:00", "+6:15", "+6:30", "+6:45",
    "+7:00", "+7:15", "+7:30", "+7:45",
    "+8:00", "+8:15", "+8:30", "+8:45",
    "+9:00", "+9:15", "+9:30", "+9:45",
    "+10:00", "+10:15", "+10:30", "+10:45",
    "+11:00", "+11:15", "+11:30", "+11:45",
    "+12:00", "+12:15", "+12:30", "+12:45",
    "+13:00", "+13:15", "+13:30", "+13:45",
    "+14:00", "+14:15", "+14:30", "+14:45",
    "+15:00", "+15:15", "+15:30", "+15:45",
    "+16:00", "+16:15", "+16:30", "+16:45",
    "+17:00", "+17:15", "+17:30", "+17:45",
    "+18:00", "+18:15", "+18:30", "+18:45",
    "+19:00", "+19:15", "+19:30", "+19:45",
    "+20:00", "+20:15", "+20:30", "+20:45",
    "+21:00", "+21:15", "+21:30", "+21:45",
    "+22:00", "+22:15", "+22:30", "+22:45",
    "+23:00", "+23:15", "+23:30", "+23:45",
    "+24:00"
};
const float jjy_dtz_value[JJY_DTZ_COUNT] = {
    -24.0f,
    -23.75f, -23.5f, -23.25f, -23.0f,
    -22.75f, -22.5f, -22.25f, -22.0f,
    -21.75f, -21.5f, -21.25f, -21.0f,
    -20.75f, -20.5f, -20.25f, -20.0f,
    -19.75f, -19.5f, -19.25f, -19.0f,
    -18.75f, -18.5f, -18.25f, -18.0f,
    -17.75f, -17.5f, -17.25f, -17.0f,
    -16.75f, -16.5f, -16.25f, -16.0f,
    -15.75f, -15.5f, -15.25f, -15.0f,
    -14.75f, -14.5f, -14.25f, -14.0f,
    -13.75f, -13.5f, -13.25f, -13.0f,
    -12.75f, -12.5f, -12.25f, -12.0f,
    -11.75f, -11.5f, -11.25f, -11.0f,
    -10.75f, -10.5f, -10.25f, -10.0f,
    -9.75f, -9.5f, -9.25f, -9.0f,
    -8.75f, -8.5f, -8.25f, -8.0f,
    -7.75f, -7.5f, -7.25f, -7.0f,
    -6.75f, -6.5f, -6.25f, -6.0f,
    -5.75f, -5.5f, -5.25f, -5.0f,
    -4.75f, -4.5f, -4.25f, -4.0f,
    -3.75f, -3.5f, -3.25f, -3.0f,
    -2.75f, -2.5f, -2.25f, -2.0f,
    -1.75f, -1.5f, -1.25f, -1.0f,
    -0.75f, -0.5f, -0.25f,
    0,
    0.25f, 0.5f, 0.75f,
    1.0f, 1.25f, 1.5f, 1.75f,
    2.0f, 2.25f, 2.5f, 2.75f,
    3.0f, 3.25f, 3.5f, 3.75f,
    4.0f, 4.25f, 4.5f, 4.75f,
    5.0f, 5.25f, 5.5f, 5.75f,
    6.0f, 6.25f, 6.5f, 6.75f,
    7.0f, 7.25f, 7.5f, 7.75f,
    8.0f, 8.25f, 8.5f, 8.75f,
    9.0f, 9.25f, 9.5f, 9.75f,
    10.0f, 10.25f, 10.5f, 10.75f,
    11.0f, 11.25f, 11.5f, 11.75f,
    12.0f, 12.25f, 12.5f, 12.75f,
    13.0f, 13.25f, 13.5f, 13.75f,
    14.0f, 14.25f, 14.5f, 14.75f,
    15.0f, 15.25f, 15.5f, 15.75f,
    16.0f, 16.25f, 16.5f, 16.75f,
    17.0f, 17.25f, 17.5f, 17.75f,
    18.0f, 18.25f, 18.5f, 18.75f,
    19.0f, 19.25f, 19.5f, 19.75f,
    20.0f, 20.25f, 20.5f, 20.75f,
    21.0f, 21.25f, 21.5f, 21.75f,
    22.0f, 22.25f, 22.5f, 22.75f,
    23.0f, 23.25f, 23.5f, 23.75f,
    24.0f
};


uint8_t jclock_value_index_uint32(const uint32_t value, const uint32_t values[], uint8_t values_count) {
    int64_t last_value = INT64_MIN;
    uint8_t index = 0;
    for (uint8_t i = 0; i < values_count; i++) {
        if ((value >= last_value) && (value <= values[i])) {
            index = i;
            break;
        }
        last_value = values[i];
    }
    return index;
}

uint8_t jclock_value_index_float(const float value, const float values[], uint8_t values_count) {
    const float epsilon = 0.01f;
    float last_value = values[0];
    uint8_t index = 0;
    for (uint8_t i = 0; i < values_count; i++) {
        if ((value >= last_value - epsilon) && (value <= values[i] + epsilon)) {
            index = i;
            break;
        }
        last_value = values[i];
    }
    return index;
}

uint8_t jclock_value_index_bool(const bool value, const bool values[], uint8_t values_count) {
    uint8_t index = 0;
    for (uint8_t i = 0; i < values_count; i++) {
        if (value == values[i]) {
            index = i;
            break;
        }
    }
    return index;
}


static void time_format_changed(VariableItem* item) {
    ClockAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, jtime_format_text[index]);
    app->clock_settings.time_format = jtime_format_value[index];
}

static void date_format_changed(VariableItem* item) {
    ClockAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, jdate_format_text[index]);
    app->clock_settings.date_format = jdate_format_value[index];
}

static void jjy_enabled_changed(VariableItem* item) {
    ClockAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, jjy_enabled_text[index]);
    app->clock_settings.jjy_enabled = jjy_enabled_value[index];
}


static void jjy_dtz_changed(VariableItem* item) {
    ClockAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, jjy_dtz_text[index]);
    app->clock_settings.jjy_dtz = jjy_dtz_value[index];
}

static ClockAppSettings* alloc_settings() {
    ClockAppSettings* app = malloc(sizeof(ClockAppSettings));
    LOAD_JCLOCK_SETTINGS(&app->clock_settings);
    app->gui = furi_record_open(RECORD_GUI);
    app->variable_item_list = variable_item_list_alloc();
    View* view = variable_item_list_get_view(app->variable_item_list);
    view_set_previous_callback(view, jclock_settings_exit);

    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(app->variable_item_list, "Clock format", JTIME_FORMAT_COUNT, time_format_changed, app);
    value_index = jclock_value_index_uint32((uint32_t)(app->clock_settings.time_format), jtime_format_value, JTIME_FORMAT_COUNT);
    //FURI_LOG_T(TAG, "Time format index: %u", value_index);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, jtime_format_text[value_index]);

    item = variable_item_list_add(app->variable_item_list, "Date format", JDATE_FORMAT_COUNT, date_format_changed, app);
    value_index = jclock_value_index_uint32((uint32_t)(app->clock_settings.date_format), jdate_format_value, JDATE_FORMAT_COUNT);
    //FURI_LOG_T(TAG, "Date format index: %u", value_index);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, jdate_format_text[value_index]);

    // JJY Enabled
    item = variable_item_list_add(app->variable_item_list, "JJY Enabled", JJY_ENABLED_COUNT, jjy_enabled_changed, app);
    value_index = jclock_value_index_uint32((uint32_t)(app->clock_settings.jjy_enabled), jjy_enabled_value, JJY_ENABLED_COUNT);
    FURI_LOG_I(TAG, "JJY ENABLED AUTO: %u", value_index);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, jjy_enabled_text[value_index]);

    // JJY dTZ
    item = variable_item_list_add(app->variable_item_list, "JJY dTZ", JJY_DTZ_COUNT, jjy_dtz_changed, app);
    value_index = jclock_value_index_float((float)(app->clock_settings.jjy_dtz), jjy_dtz_value, JJY_DTZ_COUNT);
    FURI_LOG_I(TAG, "JJY dTZ: %02d", value_index);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, jjy_dtz_text[value_index]);


    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

static void free_settings(ClockAppSettings* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    variable_item_list_free(app->variable_item_list);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    SAVE_JCLOCK_SETTINGS(&app->clock_settings);
    free(app);
}

extern int32_t jclock_settings(void* p) {
    UNUSED(p);
    ClockAppSettings* app = alloc_settings();
    view_dispatcher_run(app->view_dispatcher);
    free_settings(app);
    return 0;
}
