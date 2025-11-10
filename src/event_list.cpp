#include "event_list.h"

EventList::EventList()
{
    _count = 0;
    for (size_t i = 0; i < EVENTLIST_MAX_EVENTS; ++i)
    {
        _used[i] = false;
        // default-constructed Event is fine
    }
}

int EventList::findIndex(uint32_t tagId) const
{
    for (int i = 0; i < (int)EVENTLIST_MAX_EVENTS; ++i)
    {
        if (_used[i] && _events[i].getTagId() == tagId)
        {   
            return i;
        }
    }
    return -1;
}

bool EventList::addOrUpdate(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year)
{
    int idx = findIndex(tagId);
    if (idx >= 0) 
    {
        _events[idx].setDate(day, month, year);
        saveEvent(_events[idx], idx);
        return true; // updated
    }

    // find free slot
    for (int i = 0; i < (int)EVENTLIST_MAX_EVENTS; ++i)
    {
        if (!_used[i]) 
        {
            _events[i] = Event(tagId, day, month, year);
            _used[i] = true;
            ++_count;
            saveEvent(_events[i], i);
            return true; // added
        }
    }

    return false; // no space
}

bool EventList::remove(uint32_t tagId)
{
    int idx = findIndex(tagId);
    if (idx < 0)
    {
        return false;
    }
    _used[idx] = false;
    _events[idx] = Event(); // reset
    if (_count > 0)
    {
        --_count;
    }

    // Remove from NVS
    _prefs.begin(EVENTLIST_NVS_NAMESPACE, false);
    String key = makeNvsKey(idx);
    _prefs.remove(key.c_str());
    _prefs.remove((key + "_used").c_str());
    _prefs.putUInt("count", _count);
    _prefs.end();

    return true;
}

bool EventList::updateDate(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year)
{
    // As requested: create automatically if tag not referenced -> same as addOrUpdate
    return addOrUpdate(tagId, day, month, year);
}

bool EventList::getEvent(uint32_t tagId, Event &out)
{
    int idx = findIndex(tagId);
    if (idx < 0)
    {
        return false;
    }
    // Auto-acknowledge if event date passed; persist if changed
    unsigned int beforeMask = 0;
    for (size_t i = 0; i < Event::MAX_ALARMS; ++i)
    {
        if (_events[idx].isAlarmAcknowledged(i)) beforeMask |= (1u << i);
    }

    _events[idx].autoAcknowledgeIfEventPassed();

    unsigned int afterMask = 0;
    for (size_t i = 0; i < Event::MAX_ALARMS; ++i)
    {
        if (_events[idx].isAlarmAcknowledged(i)) afterMask |= (1u << i);
    }

    if (beforeMask != afterMask)
    {
        // persist the change
        saveEvent(_events[idx], idx);
    }

    out = _events[idx];
    return true;
}

void EventList::clear(bool clearStorage)
{
    for (int i = 0; i < (int)EVENTLIST_MAX_EVENTS; ++i) 
    {
        _used[i] = false;
        _events[i] = Event();
    }
    _count = 0;

    if (clearStorage) 
    {
        _prefs.begin(EVENTLIST_NVS_NAMESPACE, false);
        _prefs.clear();
        _prefs.end();
    }
}

String EventList::makeNvsKey(size_t index) const 
{
    return String(EVENTLIST_NVS_KEY_PREFIX) + String(index);
}

void EventList::saveEvent(const Event& event, size_t index) const 
{
    _prefs.begin(EVENTLIST_NVS_NAMESPACE, false);
    String key = makeNvsKey(index);
    String eventStr = event.toString();
    _prefs.putString(key.c_str(), eventStr);
    _prefs.putBool((key + "_used").c_str(), true);
    _prefs.end();
}

bool EventList::loadEvent(size_t index, Event& event) const 
{
    _prefs.begin(EVENTLIST_NVS_NAMESPACE, true);
    String key = makeNvsKey(index);
    
    if (!_prefs.getBool((key + "_used").c_str(), false)) {
        _prefs.end();
        return false;
    }

    String eventStr = _prefs.getString(key.c_str(), "");
    _prefs.end();

    return Event::fromString(eventStr, event);
}

bool EventList::save() const 
{
    _prefs.begin(EVENTLIST_NVS_NAMESPACE, false);
    _prefs.clear(); // Reset all to ensure clean state

    for (size_t i = 0; i < EVENTLIST_MAX_EVENTS; ++i) 
    {
        if (_used[i]) 
        {
            saveEvent(_events[i], i);
        }
    }
    
    _prefs.putUInt("count", _count);
    _prefs.end();
    return true;
}

bool EventList::load() 
{
    clear(false); // Clear memory but not storage
    
    _prefs.begin(EVENTLIST_NVS_NAMESPACE, true);
    _count = _prefs.getUInt("count", 0);
    _prefs.end();

    for (size_t i = 0; i < EVENTLIST_MAX_EVENTS; ++i) 
    {
        Event evt;
        if (loadEvent(i, evt)) 
        {
            _events[i] = evt;
            _used[i] = true;
        }
        else 
        {
            _used[i] = false;
        }
    }

    return true;
}

size_t EventList::countActiveAlarms() const
{
    size_t totalActiveAlarms = 0;
    
    for (size_t i = 0; i < EVENTLIST_MAX_EVENTS; i++)
    {
        if (_used[i])  // Only check used slots
        {
            totalActiveAlarms += _events[i].getActiveAlarmsCount();
        }
    }
    
    return totalActiveAlarms;
}
