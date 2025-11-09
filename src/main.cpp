#include "WifiManager.h"
#include "HardwareSerial.h"
#include "nvs_flash.h"
#include "led.h"
#include "rotary_coder.h"
#include "nixies_drv.h"
#include "tags.h"
#include "WebServer.h"


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


void tagCallback(uint32_t tag_id)
{
    // If tag_id is 0, no tag is present (just removed)
    Serial.print("Tag Id:");
    Serial.println(tag_id, HEX);
}

void setup()
{
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


  wifiManager.begin();
  Serial.begin(115200);
  Serial.println("Start");
  
  // Start web server after WiFi is initialized
  webServer.begin();

  

}

void loop()
{
  
  static unsigned long lastFetch = 0;
  static int rot1Prev = 0;
  static int rot2Prev = 0;

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
