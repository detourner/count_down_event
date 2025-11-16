#pragma once
#include <Preferences.h>

class PreferencesManager
{
public:
    PreferencesManager();
    
    // Load brightness and timeout from NVS
    void load();
    
    // Save brightness and timeout to NVS
    void save();
    
    // Getters
    uint16_t getBrightness() const { return _brightness; }
    uint32_t getTimeout() const { return _timeout; }
    
    // Setters
    void setBrightness(uint16_t value) { _brightness = value; }
    void setTimeout(uint32_t value) { _timeout = value; }
    
private:
    uint16_t _brightness;
    uint32_t _timeout;
    Preferences _prefs;
};
