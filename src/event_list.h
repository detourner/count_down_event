#ifndef _EVENT_LIST_H_
#define _EVENT_LIST_H_

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "event.h"

// Maximum number of events that can be stored. Adjust as needed.
#define EVENTLIST_MAX_EVENTS 20

// Name of the NVS namespace for events
#define EVENTLIST_NVS_NAMESPACE "events"
// Key prefix for events in NVS
#define EVENTLIST_NVS_KEY_PREFIX "evt_"

class EventList 
{
public:
    // Initialize with optional auto-load from NVS
    EventList();


    // Add a new event with only tag id; date is unset (0/0/0).
    // Returns true on success, false if the list is full.
    bool addOrUpdate(uint32_t tagId);

    // Add a new event or update the date if the tag already exists.
    // Returns true on success, false if the list is full.
    bool addOrUpdate(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year);

    // Add or update an event; if hasDate is false, date is unset (0/0/0).
    // Returns true on success, false if the list is full.
    bool addOrUpdate(uint32_t tagId, bool hasDate, uint8_t day, uint8_t month, uint16_t year);

    // Remove an event by tag id. Returns true if found and removed.
    bool remove(uint32_t tagId);

    // Update the date of an existing event. If the tag does not exist,
    // a new event is created (per requirement). Returns true on success.
    bool updateDate(uint32_t tagId, uint8_t day, uint8_t month, uint16_t year);

    // Retrieve an event by tag id. Returns true if found and fills 'out'.
    // This method may update the event (auto-acknowledge) and persist changes.
    bool getEvent(uint32_t tagId, Event &out);

    // Update an event title. Returns true if event found and updated.
    bool setTitle(uint32_t tagId, const String &title);

    // Configure an alarm for an event. index 0..MAX_ALARMS-1
    bool setAlarm(uint32_t tagId, size_t index, int16_t daysBefore);

    void anyEventOnDesk();
    
    // Number of stored events
    size_t count() const { return _count; }

    // Clear all events (and optionally NVS storage)
    void clear(bool clearStorage = true);

    // Load events from NVS storage
    bool load();

    // Save current events to NVS storage
    bool save() const;

    

    // Count total number of active alarms across all events
    size_t countActiveAlarms() const;

        // Serialize all events to a JSON string
        // Format: {"events": [{...}, {...}, ...]}
        String toJson() const;

    private:
    int findIndex(uint32_t tagId) const;
    void saveEvent(const Event& event, size_t index) const;
    bool loadEvent(size_t index, Event& event) const;
    String makeNvsKey(size_t index) const;


    Event _events[EVENTLIST_MAX_EVENTS];
    bool  _used[EVENTLIST_MAX_EVENTS];
    size_t _count;
    mutable Preferences _prefs;  // For NVS access
};

#endif
