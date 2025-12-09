#ifndef _EVENT_H_
#define _EVENT_H_

#include <Arduino.h>
#include "tags.h"

class Event 
{
public:
    Event();
    Event(uint32_t tagId);
    Event(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year);

    // Getters
    uint32_t getTagId() const;
    uint8_t  getDay() const;
    uint8_t  getMonth() const;
    uint16_t getYear() const;

    // Setters
    void setTagId(uint32_t id);
    void setDate(uint8_t day, uint8_t month, uint16_t year);
    void setTitle(const String &title);

    // Getters for title
    String getTitle() const;

    // Serialize to JSON string
    // Format: {"tagId": uint32, "day": uint8, "month": uint8, "year": uint16, 
    //          "title": string, "alarms": [{...}], "status": uint}
    String toString() const;

    // Parse from JSON string produced by toString(). Returns true on success.
    static bool fromString(const String &s, Event &out);

    // Helpers
    bool matchesTag(uint32_t id) const;

    // Calculate days remaining until event (negative if event has passed)
    // Uses ESP32's RTC for current date
    int32_t getDaysRemaining() const;

    // Alarms: up to 3 alarms per event. Each alarm is defined as N days
    // before the event date. A negative value means the alarm is disabled.
    static const size_t MAX_ALARMS = 3;

    // Event status enum
    typedef enum
    {
        EVENT_STATUS_NO_CONFIGURED = 0x1000,
        EVENT_STATUS_IN_PROGRESS   = 0x2000,
        EVENT_STATUS_END           = 0x3000
    } eventStatus_t;

    // Configure an alarm (index 0..MAX_ALARMS-1). daysBefore < 0 disables it.
    bool setAlarm(size_t index, int16_t daysBefore);

    // Return number of active (due and not acknowledged) alarms for this event.
    // An alarm is considered due when daysRemaining() <= daysBefore and
    // the alarm is enabled and not yet acknowledged.
    size_t getActiveAlarmsCount() const;

    // Event status accessors
    eventStatus_t getStatus() const;
    void setStatus(eventStatus_t st);

    // Recompute status based on current date and update internal state.
    // This will set NO_CONFIGURED if date is not set, IN_PROGRESS if date
    // is in the future, and END if the event date has passed or is today.
    void updateStatus();

    // Acknowledge all alarms for this event. Alarms are also automatically
    // acknowledged when the event date is reached (daysRemaining() <= 0).
    void acknowledgeAlarms();

    // Automatically acknowledge alarms if the event date has been reached.
    // This updates the internal ack flags and should be called by managers
    // that want to persist that change.
    void autoAcknowledgeIfEventPassed();

    // Get alarm parameters for reading
    int16_t getAlarmDays(size_t index) const;
    bool isAlarmAcknowledged(size_t index) const;

    // On-desk status
    void setOnDesk(bool onDesk) { _isOnDesk = onDesk; }
    bool isOnDesk() const { return _isOnDesk; }

private:
    // Convert date to total days since epoch (2000-01-01)
    uint32_t dateToDays() const;
    static uint32_t dateToDays(uint16_t year, uint8_t month, uint8_t day);
    
    // Common initialization used by constructors
    void initAlarms();
    
    static const size_t MAX_TITLE_LEN = 64;

    uint32_t _tagId;
    uint8_t  _day;
    uint8_t  _month;
    uint16_t _year;
    String   _title;

    int16_t _alarmDays[MAX_ALARMS];   // -1 = disabled, otherwise days before
    bool    _alarmAck[MAX_ALARMS];
    
    // Current status of the event
    eventStatus_t _status;
    bool _isOnDesk = false;
};

#endif
