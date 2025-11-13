#include "event.h"
#include <ArduinoJson.h>

Event::Event()
    : _tagId(0), _day(0), _month(0), _year(0), _status(EVENT_STATUS_NO_CONFIGURED)
{
    initAlarms();
}

Event::Event(uint32_t tagId)
    : _tagId(tagId), _day(0), _month(0), _year(0), _status(EVENT_STATUS_NO_CONFIGURED)
{
    initAlarms();
}

Event::Event(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year)
    : _tagId(tagId), _day(day), _month(month), _year(year), _status(EVENT_STATUS_NO_CONFIGURED)
{
    initAlarms();
    // compute status based on provided date
    updateStatus();
}

// Common initialization used by constructors
void Event::initAlarms()
{
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        _alarmDays[i] = -1;
        _alarmAck[i] = false;
    }
}

uint32_t Event::getTagId() const { return _tagId; }
uint8_t  Event::getDay() const { return _day; }
uint8_t  Event::getMonth() const { return _month; }
uint16_t Event::getYear() const { return _year; }

void Event::setTagId(uint32_t id) { _tagId = id; }
void Event::setDate(uint8_t day, uint8_t month, uint16_t year)
{
    _day = day;
    _month = month;
    _year = year;
    // Recompute status whenever date is changed
    updateStatus();
}

void Event::setTitle(const String &title)
{
    // Limit title to MAX_TITLE_LEN characters
    if (title.length() > MAX_TITLE_LEN)
    {
        _title = title.substring(0, MAX_TITLE_LEN);
    }
    else
    {
        _title = title;
    }
}

String Event::getTitle() const
{
    return _title;
}

String Event::toString() const
{
    // Return JSON string representation
    JsonDocument doc;
    doc["tagId"] = _tagId;
    doc["day"] = _day;
    doc["month"] = _month;
    doc["year"] = _year;
    doc["title"] = _title;
    
    // Serialize alarms array
    JsonArray alarmsArray = doc["alarms"].to<JsonArray>();
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        JsonObject alarm = alarmsArray.add<JsonObject>();
        alarm["index"] = i;
        alarm["daysBefore"] = _alarmDays[i];
        alarm["acknowledged"] = _alarmAck[i];
    }
    
    doc["status"] = (unsigned int)_status;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

bool Event::fromString(const String &s, Event &out)
{
    // Parse JSON string
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, s);
    
    if (error) {
        return false;  // JSON parsing failed
    }
    
    // Check required fields using is<T>() instead of deprecated containsKey()
    if (!doc["tagId"].is<uint32_t>() || !doc["day"].is<uint8_t>() || 
        !doc["month"].is<uint8_t>() || !doc["year"].is<uint16_t>()) {
        return false;  // Missing required fields
    }
    
    uint32_t tagId = doc["tagId"];
    uint8_t day = doc["day"];
    uint8_t month = doc["month"];
    uint16_t year = doc["year"];
    
    if (day <= 0 || month <= 0 || year <= 0) {
        return false;  // Invalid date
    }
    
    out = Event(tagId, day, month, year);
    
    // Parse optional title
    if (doc["title"].is<String>()) {
        out.setTitle(doc["title"].as<String>());
    }
    
    // Initialize alarms
    for (size_t i = 0; i < Event::MAX_ALARMS; ++i) {
        out._alarmDays[i] = -1;
        out._alarmAck[i] = false;
    }
    
    // Parse alarms array if present
    if (doc["alarms"].is<JsonArray>()) {
        JsonArray alarmsArray = doc["alarms"].as<JsonArray>();
        size_t idx = 0;
        for (JsonObject alarm : alarmsArray) {
            if (idx >= Event::MAX_ALARMS) break;
            
            if (alarm["daysBefore"].is<int16_t>()) {
                out._alarmDays[idx] = alarm["daysBefore"];
            }
            if (alarm["acknowledged"].is<bool>()) {
                out._alarmAck[idx] = alarm["acknowledged"];
            }
            idx++;
        }
    }
    
    // Parse optional status
    if (doc["status"].is<uint32_t>()) {
        unsigned int st = doc["status"];
        out._status = (eventStatus_t)st;
    } else {
        out.updateStatus();
    }
    
    return true;
}

bool Event::matchesTag(uint32_t id) const { return _tagId == id; }

uint32_t Event::dateToDays(uint16_t year, uint8_t month, uint8_t day)
{
    // Calculation based on Zeller's algorithm, adapted for day counting
    // Epoch: 2000-01-01 (compatible with ESP32's time.h)
    
    if (month <= 2)
    {
        year--;
        month += 12;
    }
    
    uint32_t days = 365 * year;              // Days from complete years
    days += year / 4;                        // Add leap years
    days -= year / 100;                      // Century correction
    days += year / 400;                      // Leap century correction
    days += (153 * month - 457) / 5;        // Days from months (Zeller's formula)
    days += day - 1;                         // Days from current month
    
    // Adjust relative to epoch 2000-01-01
    const uint32_t days2000 = 730484;        // Days between 0000-01-01 and 2000-01-01
    if (days >= days2000)
    {
        days -= days2000;
    }
    
    return days;
}

uint32_t Event::dateToDays() const
{
    return dateToDays(_year, _month, _day);
}

int32_t Event::getDaysRemaining() const
{
    time_t now;
    struct tm timeinfo;
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    // Convert current date and event date to days since epoch
    uint32_t currentDays = dateToDays(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday
    );
    
    uint32_t eventDays = dateToDays();
    
    // Calculate difference (negative if event has passed)
    int32_t diff = static_cast<int32_t>(eventDays) - static_cast<int32_t>(currentDays);
    return diff;
}

void Event::autoAcknowledgeIfEventPassed()
{
    int32_t diff = getDaysRemaining();
    if (diff <= 0)
    {
        for (size_t i = 0; i < MAX_ALARMS; ++i)
        {
            _alarmAck[i] = true;
        }
    }
}

Event::eventStatus_t Event::getStatus() const
{
    return _status;
}

void Event::setStatus(Event::eventStatus_t st)
{
    _status = st;
}

void Event::updateStatus()
{
    // If no valid date configured
    if (_day == 0 || _month == 0 || _year == 0)
    {
        _status = Event::EVENT_STATUS_NO_CONFIGURED;
        return;
    }

    int32_t daysLeft = getDaysRemaining();
    if (daysLeft <= 0)
    {
        _status = Event::EVENT_STATUS_END;
        // mark alarms acknowledged when event passed
        for (size_t i = 0; i < MAX_ALARMS; ++i) _alarmAck[i] = true;
    }
    else
    {
        _status = Event::EVENT_STATUS_IN_PROGRESS;
    }
}

bool Event::setAlarm(size_t index, int16_t daysBefore)
{
    if (index >= MAX_ALARMS) return false;
    _alarmDays[index] = daysBefore;
    _alarmAck[index] = false;
    return true;
}

size_t Event::getActiveAlarmsCount() const
{
    int32_t daysLeft = getDaysRemaining();
    size_t count = 0;
    if (daysLeft <= 0) return 0; // already auto-acked
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        if (_alarmDays[i] >= 0 && !_alarmAck[i])
        {
            if (daysLeft <= _alarmDays[i])
            {
                ++count; // due now or overdue relative to alarm threshold
            }
        }
    }
    return count;
}

void Event::acknowledgeAlarms()
{
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        _alarmAck[i] = true;
    }
}

int16_t Event::getAlarmDays(size_t index) const
{
    if (index >= MAX_ALARMS) return -1;
    return _alarmDays[index];
}

bool Event::isAlarmAcknowledged(size_t index) const
{
    if (index >= MAX_ALARMS) return true;
    return _alarmAck[index];
}

// New status field default handling: if date not set -> NO_CONFIGURED
