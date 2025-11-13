#include "WifiManager.h"
#include "nvs_flash.h"
#include "led.h"
#include "RotaryEncoder.h"
#include "nixies_drv.h"
#include "tags.h"
#include "timeout.h"
#include "WebServer.h"
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

Led wink_led;
RotaryEncoder  *Rot1 = nullptr;
RotaryEncoder  *Rot2 = nullptr;

NixiesDriver nixies;
TimeOut timeOut;

tags tag;
WebServer webServer;
EventList events;  // Will auto-load from NVS

unsigned long startTimeDisplay = 0;

IRAM_ATTR void checkPositionRot1()
{
  Rot1->tick(); // just call tick() to check the state.
}

IRAM_ATTR void checkPositionRot2()
{
  Rot2->tick(); // just call tick() to check the state.
}

// Function to update LED winks based on active alarms
void updateActiveAlarmsWinks()
{
    // Get total number of active alarms from EventList
    size_t totalActiveAlarms = events.countActiveAlarms();
    
    // Update LED winks based on total number of active alarms
    wink_led.setWinks(totalActiveAlarms);
}

void timeOutCallback(uint32_t dispValue)
{
    Serial.print("Display Value due to timeout: ");
    Serial.println(dispValue);
    if(dispValue == 0)
    {
        nixies.SetBlink(1000,0); // off !
    }
    else if(dispValue == timeOut.Infinity())
    {
        nixies.SetBlink(250,0); // blink fast !
        nixies.DispValue(999);
    }
    else
    {
        nixies.SetBlink(500,50); // blink low !
        nixies.DispValue(dispValue);
    }
}

void tagCallback(uint32_t tag_id)
{
  timeOut.newEvent(); // Reset timeout !

  // If tag_id is 0, no tag is present (just removed)
  if (tag_id == 0)
  {
      Serial.println("Tag removed");
      return;
  }

  Serial.print("Tag Id: ");
  Serial.println(tag_id, HEX);

  // Try to get existing event
  Event evt;

  // If not found, create a new event
  if(events.getEvent(tag_id, evt) == false)
  {
    Serial.println("Event not found, will create a new one.");
    // Proceed to create a new event below 
    if (events.addOrUpdate(tag_id))
    {
      Serial.println("New event created");
    }
    else
    {
      Serial.println("Error: Could not create event (storage full)");
    }
  }

  if (events.getEvent(tag_id, evt))
  {
    evt.acknowledgeAlarms(); // Acknowledge any due alarms
    updateActiveAlarmsWinks();  // Update winks when tag is removed
    timeOut.newEvent(); // Reset timeout !

    // Event exists, display days remaining
    int32_t daysLeft = evt.getDaysRemaining();
    Serial.print("Days remaining: ");
    Serial.println(daysLeft);
    switch(evt.getStatus())
    {
      case Event::EVENT_STATUS_NO_CONFIGURED:
        Serial.println("Event status: NO_CONFIGURED");
        nixies.DispValue(999);
        nixies.SetBlink(1000,50);
        break;
      case Event::EVENT_STATUS_IN_PROGRESS:
        Serial.println("Event status: IN_PROGRESS");
        nixies.DispValue(daysLeft);
        nixies.SetBlink(0,0); // no blink
        break;
      case Event::EVENT_STATUS_END:
        Serial.println("Event status: END");
        nixies.DispValue(0);
        nixies.SetBlink(1000,50);
        break;
      default:
        Serial.println("Event status: UNKNOWN");
        break;
    }
  }
  else
  {
    Serial.println("Error retrieving or creating event.");
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  // clear the NVS partition (and all preferences stored in it)
  //nvs_flash_erase(); // erase the NVS partition and...
  //nvs_flash_init(); // initialize the NVS partition.

  events.load(); // Load events from NVS

  timeOut.Setup(10000, 600000, &timeOutCallback);


  // Rotary Encoder for Display Duration. Define limits and initial value.
  // The duration is in seconds.
  Rot1 = new RotaryEncoder(PinRot1_1, PinRot1_2, RotaryEncoder::LatchMode::TWO03);
  Rot1->setMin(10);
  Rot1->setMax(nixies.GetMaxBrightness()/50);
  Rot1->setPosition(nixies.GetMaxBrightness()/50); // initial 5 minutes

  // register interrupt routine
  attachInterrupt(digitalPinToInterrupt(PinRot1_1), checkPositionRot1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PinRot1_2), checkPositionRot1, CHANGE);
  

  
  /*Rot1.setMin(10);
  Rot1.setMax(nixies.GetMaxBrightness()/50);
  Rot1.setVal(nixies.GetMaxBrightness()/50);

  Rot2.init(PinRot2_1,PinRot2_2);
  Rot2.setMin(timeOut.getPosMin());
  Rot2.setMax(timeOut.getPosMax());
  Rot2.setVal(timeOut.getPosDef());*/

  Rot2 = new RotaryEncoder(PinRot2_1, PinRot2_2, RotaryEncoder::LatchMode::TWO03);

  Rot2->setMin(timeOut.getPosMin());
  Rot2->setMax(timeOut.getPosMax());
  Rot2->setPosition(timeOut.getPosDef());

  // register interrupt routine
  attachInterrupt(digitalPinToInterrupt(PinRot2_1), checkPositionRot2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PinRot2_2), checkPositionRot2, CHANGE);
  
    
  wink_led.Setup(PinWinkLed);
  wink_led.setWinks( 0 );

  



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
  }

  if(Rot1->getPosition() != rot1Prev)
  {
    Serial.print("Rot1: ");
    rot1Prev = Rot1->getPositionWithLimit();
    //timeOut.newEvent();
    Serial.println(rot1Prev);    

  }

  if(Rot2->getPosition() != rot2Prev)
  {
    Serial.print("Rot2: ");
    rot2Prev = Rot2->getPositionWithLimit();
    //timeOut.newEvent();
    Serial.println(rot2Prev);    

  }

  if(timeOut.getDispStatus())
  {
    
    //nixies.SetBrightness(Rot1->getPosition()*50);
  }
  else
  {
    
    nixies.SetBrightness(0); // off !
  }

  wink_led.CyclTask();
  nixies.CyclTask();
  tag.CyclTask();

  timeOut.CyclTask(Rot2->getPosition());

  Rot1->tick();
  Rot2->tick();



}
