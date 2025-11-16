#include "preferences_manager.h"

#define PREFS_NAMESPACE "app_settings"
#define PREFS_KEY_BRIGHTNESS "brightness"
#define PREFS_KEY_TIMEOUT "timeout"

// Default values
#define DEFAULT_BRIGHTNESS 200
#define DEFAULT_TIMEOUT 300  // 5 minutes in seconds

PreferencesManager::PreferencesManager()
    : _brightness(DEFAULT_BRIGHTNESS), _timeout(DEFAULT_TIMEOUT)
{
}

void PreferencesManager::load()
{
    _prefs.begin(PREFS_NAMESPACE, true);  // Read-only mode
    _brightness = _prefs.getUInt(PREFS_KEY_BRIGHTNESS, DEFAULT_BRIGHTNESS);
    _timeout = _prefs.getUInt(PREFS_KEY_TIMEOUT, DEFAULT_TIMEOUT);
    _prefs.end();
}

void PreferencesManager::save()
{
    _prefs.begin(PREFS_NAMESPACE, false);  // Write mode
    _prefs.putUInt(PREFS_KEY_BRIGHTNESS, _brightness);
    _prefs.putUInt(PREFS_KEY_TIMEOUT, _timeout);
    _prefs.end();
}
