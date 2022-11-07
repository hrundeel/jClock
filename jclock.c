/*
* jClock - basic clock with clock synchronization protocol JJY
* Author: hrundeel (https://gist.github.com/hrundeel)
* Added:
*   Options for JJY disable/enable.
*   Options for JJY delta TZ (if your current watch region don't match flipper time).
*   Reworked getting rtc time/update local values.
*   Reworked main screen (+charge/jjy/tx status icons, +charge%%).
* Based on:
*   Simple clock for Flipper Zero by CompaqDisc (https://gist.github.com/CompaqDisc, https://gist.github.com/CompaqDisc/4e329c501bd03c1e801849b81f48ea61)
*   Timer by GMMan (?)
*   Settings by kowalski7cc (https://gist.github.com/kowalski7cc)
*/

#include <furi.h>
#include <furi_hal.h>

#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>

#include <gui/gui.h>
#include <gui/elements.h>
//#include <gui/icon.h>

#include "jclock.h"
#include "jclock_settings.h"
#include "compiled/jclock_icons.h"

//JJY 0..5hrs +3MSK?
#define DTZ_LEN 9


// correct time to dTZ (-24f..+24f)
static void jjy_correct_dtz(ClockState* state) {
    //furi_hal_rtc_get_datetime(&curr_dt);
    state->JJYtimestamp = (double)furi_hal_rtc_datetime_to_timestamp(&state->datetime) + ((double)state->settings.jjy_dtz * 86400);
    //check to crossing daylight saving time?
    //!!! add function when will be available
    //state->JJYdatetime = *localtime(&corrected_timestamp);
    return;
}

// transmit current signal to antenna
static void jjy_transmit(ClockState* state) {

    //const GpioPin gpio_nfc_irq_rfid_pull = {.port = RFID_PULL_GPIO_Port, .pin = RFID_PULL_Pin};
    //const GpioPin gpio_rfid_carrier_out = {.port = RFID_OUT_GPIO_Port, .pin = RFID_OUT_Pin};
    //const GpioPin gpio_rfid_data_in = {.port = RFID_RF_IN_GPIO_Port, .pin = RFID_RF_IN_Pin};
    //const GpioPin gpio_rfid_carrier = {.port = RFID_CARRIER_GPIO_Port, .pin = RFID_CARRIER_Pin};

        //furi_hal_gpio_write(&gpio_ext_pc0, false);
    //furi_hal_gpio_init(&gpio_ext_pa7, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    jjy_correct_dtz(state);

    //пока просто помигаем
    if (state->datetime.second & 1) {
        // release
        //furi_hal_ibutton_pin_high();
        furi_hal_gpio_write(&gpio_ext_pc0, true);
        //furi_delay_us(OWH_WRITE_1_RELEASE);
    }
    else {
        // drive low
        //furi_hal_ibutton_pin_low();
        furi_hal_gpio_write(&gpio_ext_pc0, false);
        //furi_delay_us(OWH_WRITE_1_DRIVE);
    }
    return;
}


static void jclock_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    PluginEvent event = { .type = EventTypeKey, .input = *input_event };
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void jclock_render_callback(Canvas* const canvas, void* ctx) {
    //canvas_clear(canvas);
    //canvas_set_color(canvas, ColorBlack);

    ClockState* state = ctx;
    if (furi_mutex_acquire(state->mutex, 200) != FuriStatusOk) {
        //FURI_LOG_D(TAG, "Can't obtain mutex, requeue render");
        PluginEvent event = { .type = EventTypeTick };
        furi_message_queue_put(state->event_queue, &event, 0);
        return;
    }

    char time_string[TIME_LEN];
    char date_string[DATE_LEN];
    char meridian_string[MERIDIAN_LEN];
    char timer_string[20];

    char dtz_string[DTZ_LEN];

    //FuriHalRtcDateTime curr_dt;
    //furi_hal_rtc_get_datetime(&curr_dt);
    //uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

    //furi_hal_rtc_get_datetime(&state->datetime);
    //state->timestamp = furi_hal_rtc_datetime_to_timestamp(&state->datetime);


    // Check JJY mode
    if (furi_hal_power_is_charging()) {
        // check options for auto enabled?
        if (state->JJYmode == JJY_AUTO_ENABLED) {
#if (!DEBUG_JCLOCK) 
            if (state->datetime.hour < 5) {
#endif
                state->JJYmode = JJY_AUTO_TRANSMIT;
                FURI_LOG_I(TAG, "JJY-A Tx (charge on");
#if (!DEBUG_JCLOCK) 
            }
#endif
        }

    }
    else { // not charging (just now?)
        if (state->JJYmode == JJY_AUTO_TRANSMIT) {
            if (state->settings.jjy_enabled) {
                state->JJYmode = JJY_AUTO_ENABLED; // check if options ok
                FURI_LOG_I(TAG, "JJY-A (charge off");
            }
        }
        else {
            if (state->JJYmode != JJY_FORCED) {
                state->JJYmode = JJY_NONE; // check if options ok
                FURI_LOG_I(TAG, "JJY disabled");
            }
        }
    }

    // secondary clock functions (display clock/timer)
    if (state->settings.time_format == H24) {
        snprintf(
            time_string,
            TIME_LEN,
            JCLOCK_TIME_FORMAT,
            state->datetime.hour,
            state->datetime.minute,
            state->datetime.second);
    }
    else {
        bool pm = state->datetime.hour > 12;
        bool pm12 = state->datetime.hour >= 12;
        snprintf(
            time_string,
            TIME_LEN,
            JCLOCK_TIME_FORMAT,
            pm ? state->datetime.hour - 12 : state->datetime.hour,
            state->datetime.minute,
            state->datetime.second);

        snprintf(
            meridian_string,
            MERIDIAN_LEN,
            MERIDIAN_FORMAT,
            pm12 ? MERIDIAN_STRING_PM : MERIDIAN_STRING_AM);
    }

    if (state->settings.date_format == Iso) {
        snprintf(
            date_string, DATE_LEN, CLOCK_ISO_DATE_FORMAT, state->datetime.year, state->datetime.month, state->datetime.day);
    }
    else {
        snprintf(
            date_string, DATE_LEN, CLOCK_RFC_DATE_FORMAT, state->datetime.day, state->datetime.month, state->datetime.year);
    }

    bool timer_running = state->timer_running;
    uint32_t timer_start_timestamp = state->timer_start_timestamp;
    uint32_t timer_stopped_seconds = state->timer_stopped_seconds;

    furi_mutex_release(state->mutex);

    canvas_set_font(canvas, FontBigNumbers);

    if (timer_start_timestamp != 0) {   // timer

        int32_t elapsed_secs = timer_running ? (state->timestamp - timer_start_timestamp) :
            timer_stopped_seconds;
        snprintf(timer_string, 20, "%.2ld:%.2ld", elapsed_secs / 60, elapsed_secs % 60);
        canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, time_string); // DRAW TIME
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, timer_string); // DRAW TIMER
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignTop, date_string); // DRAW DATE
        elements_button_left(canvas, "Reset");

    }
    else {  // main

        canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignCenter, time_string);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignTop, date_string);

        if (state->settings.time_format == H12)
            canvas_draw_str_aligned(canvas, 65, 12, AlignCenter, AlignCenter, meridian_string);
        //Battery%


        snprintf(dtz_string, DTZ_LEN, "%02d%%", furi_hal_power_get_pct());
        canvas_draw_str_aligned(canvas, 128, 0, AlignRight, AlignTop, dtz_string); // DRAW dTZ

        // JJY indication
        switch (state->JJYmode) {
        case JJY_NONE:
            if (furi_hal_power_is_charging()) {
                canvas_draw_icon(canvas, 0, 0, &I_jjy_charge_7px);
            }
            break;
        case JJY_AUTO_ENABLED:
            //canvas_draw_icon(canvas, canvas_width(canvas) - icon_get_width(&I_jjy_auto_7px), 0, &I_jjy_auto_7px);
            canvas_draw_icon(canvas, 0, 0, &I_jjy_auto_7px);
            break;

        case JJY_AUTO_TRANSMIT:
            //canvas_draw_icon(canvas, canvas_width(canvas) - icon_get_width(&I_jjy_auto_transmit_7px), 0, &I_jjy_auto_transmit_7px);
            canvas_draw_icon(canvas, 0, 0, &I_jjy_auto_transmit_7px);
            break;

        case JJY_FORCED:
            //canvas_draw_icon(canvas, canvas_width(canvas) - icon_get_width(&I_jjy_forced_7px), 0, &I_jjy_forced7px);
            canvas_draw_icon(canvas, 0, 0, &I_jjy_forced_7px);
            break;

        default:
            break;
        }

        // JJY dTZ
        if (state->JJYmode != JJY_NONE) {
            uint8_t  value_index = jclock_value_index_float((float)state->settings.jjy_dtz, jjy_dtz_value, JJY_DTZ_COUNT);
            FURI_LOG_I(TAG, "JJY F dTZ: %02d", value_index);
            //snprintf(dtz_string, DTZ_LEN, "%02d:%02d", (int8_t)state->settings.jjy_dtz, abs((int16_t)(state->settings.jjy_dtz * 100) % 100));
            //snprintf(dtz_string, DTZ_LEN, "%s", (const char*)item);
            snprintf(dtz_string, DTZ_LEN, "%s", jjy_dtz_text[value_index]);

            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 12, 0, AlignLeft, AlignTop, dtz_string); // DRAW dTZ
        }

    }

    if (timer_running) {
        elements_button_center(canvas, "Stop");
    }
    else if (timer_start_timestamp != 0 && !timer_running) {
        elements_button_center(canvas, "Start");
    }

}

static void jclock_state_init(ClockState* const state) {
    LOAD_JCLOCK_SETTINGS(&state->settings);

    if (state->settings.time_format != H12 && state->settings.time_format != H24) {
        state->settings.time_format = H12;
    }
    if (state->settings.date_format != Iso && state->settings.date_format != Rfc) {
        state->settings.date_format = Iso;
    }
    FURI_LOG_D(TAG, "Time format: %s", state->settings.time_format == H12 ? "12h" : "24h");
    FURI_LOG_D(
        TAG, "Date format: %s", state->settings.date_format == Iso ? "ISO 8601" : "RFC 5322");
    //furi_hal_rtc_get_datetime(&state->datetime);

    //JJY
    FURI_LOG_I(TAG, "JJY Enabled: %s", state->settings.jjy_enabled == ENABLED ? "ON" : "OFF");
    if (state->settings.jjy_enabled == ENABLED) {
        state->JJYmode = JJY_AUTO_ENABLED;
    }
    else {
        state->JJYmode = JJY_NONE;
    }
}

// Runs every 1000ms by default
static void jclock_tick(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    PluginEvent event = { .type = EventTypeTick };
    // It's OK to loose this event if system overloaded
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t jclock(void* p) {
    UNUSED(p);
    ClockState* plugin_state = malloc(sizeof(ClockState));

    plugin_state->event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    if (plugin_state->event_queue == NULL) {
        FURI_LOG_E(TAG, "Cannot create event queue");
        free(plugin_state);
        return 255;
    }
    //FURI_LOG_D(TAG, "Event queue created");

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if (plugin_state->mutex == NULL) {
        FURI_LOG_E(TAG, "Cannot create mutex");
        furi_message_queue_free(plugin_state->event_queue);
        free(plugin_state);
        return 255;
    }
    //FURI_LOG_D(TAG, "Mutex created");

    jclock_state_init(plugin_state);

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, jclock_render_callback, plugin_state);
    view_port_input_callback_set(view_port, jclock_input_callback, plugin_state->event_queue);

    FuriTimer* timer =
        furi_timer_alloc(jclock_tick, FuriTimerTypePeriodic, plugin_state->event_queue);

    if (timer == NULL) {
        FURI_LOG_E(TAG, "Cannot create timer");
        furi_mutex_free(plugin_state->mutex);
        furi_message_queue_free(plugin_state->event_queue);
        free(plugin_state);
        return 255;
    }
    //FURI_LOG_D(TAG, "Timer created");

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_timer_start(timer, furi_kernel_get_tick_frequency());
    //FURI_LOG_D(TAG, "Timer started");

    furi_hal_rtc_get_datetime(&plugin_state->datetime);
    plugin_state->timestamp = furi_hal_rtc_datetime_to_timestamp(&plugin_state->datetime);

    //JJY init
    //furi_hal_gpio_init_simple(&gpio_ext_pc0, GpioModeOutputPushPull);
    furi_hal_gpio_init(&gpio_ext_pc0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    // Main loop
    PluginEvent event;
    for (bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(plugin_state->event_queue, &event, 100);

        if (event_status != FuriStatusOk) continue;

        if (furi_mutex_acquire(plugin_state->mutex, FuriWaitForever) != FuriStatusOk) continue;

        // press events
        if (event.type == EventTypeKey) {
            if (event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
                switch (event.input.key) {
                case InputKeyUp:
                    //plugin_state->jjy_forced = true;
                    plugin_state->JJYmode = JJY_FORCED;
                    FURI_LOG_I(TAG, "JJY-F Tx");
                    break;
                case InputKeyDown:
                    //plugin_state->jjy_forced = false;
                    // check enabled/time ok to transmit!
                    if (plugin_state->settings.jjy_enabled == ENABLED) {
                        plugin_state->JJYmode = JJY_AUTO_ENABLED;
                        FURI_LOG_I(TAG, "JJY-A");
                    }
                    else {
                        plugin_state->JJYmode = JJY_NONE;
                        FURI_LOG_I(TAG, "JJY none ");
                    }
                    break;
                case InputKeyRight:
                    break;
                case InputKeyLeft:
                    if (plugin_state->timer_start_timestamp != 0) {
                        // Reset seconds
                        plugin_state->timer_running = false;
                        plugin_state->timer_start_timestamp = 0;
                        plugin_state->timer_stopped_seconds = 0;
                    }
                    break;
                case InputKeyOk:;
                    // START/STOP TIMER

                    FuriHalRtcDateTime curr_dt;
                    furi_hal_rtc_get_datetime(&curr_dt);
                    uint32_t curr_ts = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

                    if (plugin_state->timer_running) {
                        // Update stopped seconds
                        plugin_state->timer_stopped_seconds =
                            curr_ts - plugin_state->timer_start_timestamp;
                    }
                    else {
                        if (plugin_state->timer_start_timestamp == 0) {
                            // Set starting timestamp if this is first time
                            plugin_state->timer_start_timestamp = curr_ts;
                        }
                        else {
                            // Timer was already running, need to slightly readjust so we don't
                            // count the intervening time
                            plugin_state->timer_start_timestamp =
                                curr_ts - plugin_state->timer_stopped_seconds;
                        }
                    }
                    plugin_state->timer_running = !plugin_state->timer_running;
                    break;
                case InputKeyBack:
                    // Exit the plugin
                    processing = false;
                    break;
                default:
                    break;
                }
            }
        } /*else if(event.type == EventTypeTick) {
            furi_hal_rtc_get_datetime(&plugin_state->datetime);
        }*/
        if (event.type == EventTypeTick) {
            furi_hal_rtc_get_datetime(&plugin_state->datetime);
            plugin_state->timestamp = furi_hal_rtc_datetime_to_timestamp(&plugin_state->datetime);

            //JJY transmission if enabled
            if ((plugin_state->JJYmode == JJY_AUTO_TRANSMIT) || (plugin_state->JJYmode == JJY_FORCED)) {
                jjy_transmit(plugin_state);
            }

        }
        view_port_update(view_port);
        furi_mutex_release(plugin_state->mutex);
    }

    //JJY free
    // Reset GPIO pins to default state
    furi_hal_gpio_init(&gpio_ext_pc0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(plugin_state->event_queue);
    furi_mutex_free(plugin_state->mutex);
    free(plugin_state);

    return 0;
}
