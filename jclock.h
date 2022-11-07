#pragma once

#include <gui/view.h>

#include <input/input.h>
#include "jclock_settings.h"

#define DEBUG_JCLOCK 1

#define TAG "jClock"

#define CLOCK_ISO_DATE_FORMAT "%.4d-%.2d-%.2d"
#define CLOCK_RFC_DATE_FORMAT "%.2d-%.2d-%.4d"
#define JCLOCK_TIME_FORMAT "%.2d:%.2d:%.2d"

#define MERIDIAN_FORMAT "%s"
#define MERIDIAN_STRING_AM "AM"
#define MERIDIAN_STRING_PM "PM"

#define TIME_LEN 12
#define DATE_LEN 14
#define MERIDIAN_LEN 3

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef enum { JJY_NONE = 0, JJY_AUTO_ENABLED, JJY_AUTO_TRANSMIT, JJY_FORCED } JJYmode;

typedef struct {
    ClockSettings settings;
    FuriHalRtcDateTime datetime;
    uint32_t timestamp;

    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
    uint32_t timer_start_timestamp;
    uint32_t timer_stopped_seconds;
    bool timer_running;
    JJYmode JJYmode;
    FuriHalRtcDateTime JJYdatetime;     // corrected to dTMZ time
    uint32_t JJYtimestamp;              // for correction dTMZ
    uint32_t JJYinstance;               // current stage of transmitting
} ClockState;
