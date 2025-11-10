#include "event.h"

Event::Event()
  : _tagId(0), _day(0), _month(0), _year(0)
{
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        _alarmDays[i] = -1;
        _alarmAck[i] = false;
    }
}

Event::Event(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year)
  : _tagId(tagId), _day(day), _month(month), _year(year)
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
}

static String pad2(uint8_t v)
{
    if (v < 10) 
    {
        return String("0") + String(v);
    }
    return String(v);
}

String Event::toString() const
{
    // Format: "<tagId>;<dd>/<mm>/<yyyy>"
    String s = String((unsigned long)_tagId);
    s += ";";
    s += pad2(_day);
    s += "/";
    s += pad2(_month);
    s += "/";
    s += String((unsigned int)_year);
    // Append alarms: format ;d1,d2,d3;ackMask  (d = -1 means disabled)
    s += ";";
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        if (i) s += ",";
        s += String((int)_alarmDays[i]);
    }
    unsigned int ackMask = 0;
    for (size_t i = 0; i < MAX_ALARMS; ++i)
    {
        if (_alarmAck[i]) ackMask |= (1u << i);
    }
    s += ";";
    s += String((unsigned int)ackMask);
    return s;
}

bool Event::fromString(const String &s, Event &out)
{
    int sep = s.indexOf(';');
    if (sep < 0) return false;

    String idStr = s.substring(0, sep);
    String dateStr = s.substring(sep + 1);

    // parse id using strtoul on c_str
    const char *id_c = idStr.c_str();
    char *endptr = nullptr;
    unsigned long id = strtoul(id_c, &endptr, 10);
    if (id_c == endptr) return false; // no conversion

    int p1 = dateStr.indexOf('/');
    int p2 = dateStr.lastIndexOf('/');
    if (p1 < 0 || p2 < 0 || p1 == p2) return false;

    int day = dateStr.substring(0, p1).toInt();
    int month = dateStr.substring(p1 + 1, p2).toInt();
    int year = dateStr.substring(p2 + 1).toInt();

    if (day <= 0 || month <= 0 || year <= 0) return false;

    out = Event((uint32_t)id, (uint8_t)day, (uint8_t)month, (uint16_t)year);
    // Parse optional alarms part: expected format after date: ;d1,d2,d3;ackMask
    int pos = s.indexOf(';', sep + 1);
    if (pos >= 0)
    {
        String alarmsPart = s.substring(pos + 1);
        // split by ';' to get alarms and ackmask
        int semi = alarmsPart.indexOf(';');
        String alarmsList = (semi >= 0) ? alarmsPart.substring(0, semi) : alarmsPart;
        String ackStr = (semi >= 0) ? alarmsPart.substring(semi + 1) : "0";

        // initialize alarms
        for (size_t i = 0; i < Event::MAX_ALARMS; ++i)
        {
            out._alarmDays[i] = -1;
            out._alarmAck[i] = false;
        }

        // parse alarmsList as d1,d2,d3
        int start = 0;
        for (size_t i = 0; i < Event::MAX_ALARMS; ++i)
        {
            int comma = alarmsList.indexOf(',', start);
            String tok;
            if (comma >= 0)
            {
                tok = alarmsList.substring(start, comma);
                start = comma + 1;
            }
            else
            {
                tok = alarmsList.substring(start);
            }

            tok.trim();
            if (tok.length() > 0)
            {
                int val = tok.toInt();
                out._alarmDays[i] = val;
            }
        }

        // parse ack mask
        unsigned int ackMask = (unsigned int)ackStr.toInt();
        for (size_t i = 0; i < Event::MAX_ALARMS; ++i)
        {
            out._alarmAck[i] = (ackMask & (1u << i)) != 0;
        }
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