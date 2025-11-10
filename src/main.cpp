#include "WifiManager.h"
#include "HardwareSerial.h"
#include "nvs_flash.h"
#include "led.h"
#include "rotary_coder.h"
#include "nixies_drv.h"
#include "tags.h"
#include "WebServer.h"
#include "event.h"
#include "event_list.h"
#include "time.h"
#include "esp_sntp.h"


const char* apSSID = "MazeSliderMachine";
const char* apPassword = "12345678";

WifiManager wifiManager(apSSID, apPassword);

#define PinLed 2
#define PinWinkLed 32
#define PinRot1_1  25
#define PinRot1_2  33
#define PinRot2_1  35
#define PinRot2_2  34

String inputString = "";
bool stringComplete = false;

Led wink_led;
rotaryCoder Rot1;
rotaryCoder Rot2;

NixiesDriver nixies;

tags tag;
WebServer webServer;
EventList events;  // Will auto-load from NVS

// Function to display days remaining on nixies with limits (0-999)
void displayDaysRemaining(int32_t days)
{
    // Limit the display range
    if (days < 0)
    {
        days = 0;
        nixies.SetBlink(1000, 50); // Blink to indicate event passed
    }
    else if (days > 999)
    {
        days = 999;
        nixies.SetBlink(1000, 50); // Blink to indicate event is more than displayable
    }
    else
    {
        nixies.SetBlink(0, 0); // Disable blinking for normal display
    }
    
    nixies.DispValue(days);
}

// Function to update LED winks based on active alarms
void updateActiveAlarmsWinks()
{
    // Get total number of active alarms from EventList
    size_t totalActiveAlarms = events.countActiveAlarms();
    
    // Update LED winks based on total number of active alarms
    wink_led.setWinks(totalActiveAlarms);
}

void tagCallback(uint32_t tag_id)
{
  // If tag_id is 0, no tag is present (just removed)
  if (tag_id == 0)
  {
      Serial.println("Tag removed");
      updateActiveAlarmsWinks();  // Update winks when tag is removed
      return;
  }

  Serial.print("Tag Id: ");
  Serial.println(tag_id, HEX);

  // Try to get existing event
  Event evt;
  if (events.getEvent(tag_id, evt))
  {
      evt.acknowledgeAlarms(); // Acknowledge any due alarms
      updateActiveAlarmsWinks();  // Update winks when tag is removed
      // Event exists, display days remaining
      int32_t daysLeft = evt.getDaysRemaining();
      Serial.print("Days remaining: ");
      Serial.println(daysLeft);
      displayDaysRemaining(daysLeft);
  }
  else
  {
    // New tag, create event for today + 1 year
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Set event one year from today
    if (events.addOrUpdate(tag_id, 
                          timeinfo.tm_mday,
                          timeinfo.tm_mon + 1,
                          timeinfo.tm_year + 1900 + 1))
    {
      Serial.println("New event created (1 year from today)");
      displayDaysRemaining(365);  // Approximate for display
    }
    else
    {
      Serial.println("Error: Could not create event (storage full)");
      nixies.DispValue(0);  // Error indication
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  // clear the NVS partition (and all preferences stored in it)
  //nvs_flash_erase(); // erase the NVS partition and...
  //nvs_flash_init(); // initialize the NVS partition.
    
  wink_led.Setup(PinWinkLed);
  wink_led.setWinks( 0 );

  Rot1.init(PinRot1_1,PinRot1_2);
  Rot1.setMax(100);
  Rot1.setVal(50);
  Rot1.setMin(10);

  Rot2.init(PinRot2_1,PinRot2_2);
  Rot2.setMax(200);
  Rot2.setVal(100);
  Rot2.setMin(20);

  nixies.Setup();
  nixies.SetBrightness(200);
  nixies.SetBlink(1000, 50); // blink every 1s with 50% duty cycle
  nixies.DispValue(999);

  tag.Setup(&tagCallback);

  // Initialize WiFi
  wifiManager.begin();
  
  // Start web server after WiFi is initialized
  webServer.begin();

  // Initial NTP configuration
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov", "time.google.com"); // UTC+1 offset, daylight saving enabled
}

#define NTP_CHECK_INTERVAL 10000     // Check NTP status every 10 seconds
#define NTP_RETRY_INTERVAL 3600000  // Retry full sync every hour if not synchronized
#define ALARM_CHECK_INTERVAL 10000  // Check alarms every 10 seconds


// Print current time (for debug)
void printLocalTime()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%d/%m/%Y %H:%M:%S", &timeinfo);
    Serial.println(strftime_buf);
}

// Check if time is synchronized
bool isTimeSynchronized()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo))
    {
        return false;
    }
    return timeinfo.tm_year > (2023 - 1900);
}

void loop()
{
  static unsigned long lastFetch = 0;
  static unsigned long lastNTPCheck = 0;
  static unsigned long lastNTPRetry = 0;
  static unsigned long lastAlarmCheck = 0;
  static int rot1Prev = 0;
  static int rot2Prev = 0;

  // Non-blocking NTP sync check
  unsigned long currentMillis = millis();
  if (currentMillis - lastNTPCheck >= NTP_CHECK_INTERVAL)
  {
    lastNTPCheck = currentMillis;
    // Print time every 10 seconds for debug
    printLocalTime();

    if (!isTimeSynchronized())
    {
      // Retry only if enough time elapsed since last retry
      if (currentMillis - lastNTPRetry >= NTP_RETRY_INTERVAL)
      {
        Serial.println("Retrying NTP synchronization...");
        configTime(3600, 3600, "pool.ntp.org", "time.nist.gov", "time.google.com");
        lastNTPRetry = currentMillis;
      }
      else
      {
        Serial.println("Waiting for NTP synchronization...");
      }
    }
    else
    {
      // Reset retry timer on successful sync
      lastNTPRetry = currentMillis;
      Serial.println("NTP synchronized.");
    }
  }

  // Check for active alarms every ALARM_CHECK_INTERVAL
  if (currentMillis - lastAlarmCheck >= ALARM_CHECK_INTERVAL)
  {
    lastAlarmCheck = currentMillis;
    updateActiveAlarmsWinks();
  }

  if (millis() - lastFetch > 500)
   {
    lastFetch = millis();
    if(wifiManager.checkWiFiConnection())
    {
      Serial.println("o");
    }
    else
    {
      Serial.println("-");
    }
    wink_led.setWinks(3);   
  }

  wink_led.CyclTask();
  nixies.CyclTask();
  tag.CyclTask();

  if(Rot1.getVal() != rot1Prev)
  {
      rot1Prev = Rot1.getVal();
      Serial.print("Rotary1:");
      Serial.println(rot1Prev);
  }

  if(Rot2.getVal() != rot2Prev)
  {
      rot2Prev = Rot2.getVal();
      Serial.print("Rotary2:");
      Serial.println(rot2Prev);
  }

}
