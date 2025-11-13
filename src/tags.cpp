#include "tags.h"
#include <Arduino.h>
#include "MFRC522.h"

static MFRC522       _mfrc522(SS_PIN, RST_PIN);             // Tag reader

void tags::Setup(CallbackType callback)
{
    SPI.begin();

    _mfrc522.PCD_Init();   // Init MFRC522 module
    //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

    _retry = TAGS_NUM_OF_RETRY;
    _tagIsPresent = false;
    _current_tag = 0;

    _prevTime = 0;
    _currentTime = 0;

    _callback = callback;
}

void tags::CyclTask(void)
{

    unsigned long time = millis();
    if(time - _prevTime < 50) // update every 50ms
    {
        return;
    }
    _currentTime += (time - _prevTime);
    _prevTime = time;

    if(_currentTime < 100)
    {
        return;
    }
    _currentTime = 0;

    if ( _mfrc522.PICC_IsNewCardPresent())
    {
        if ( _mfrc522.PICC_ReadCardSerial())
        {

            if(_mfrc522.uid.size == sizeof(uint32_t)) // Compatible only
            {
                uint32_t tag_id = 0;
                memcpy(&tag_id, _mfrc522.uid.uidByte, sizeof(uint32_t));
                if(tag_id != _current_tag)
                {
                        _current_tag = tag_id;
                        _tagIsPresent = true;
                       if(_callback != nullptr)
                       {
                           _callback(_current_tag);
                       }
                }
                _retry = TAGS_NUM_OF_RETRY;
            }
        }
    }
    else
    {
        if(_retry > 0)
        {
            _retry--;
        }
        else if(_retry == 0)
        {
            _retry--;
            _tagIsPresent = false;
            _current_tag = 0;
            if(_callback != nullptr)
            {
                _callback(0); // No tag
            }
        }
    }
}

void tags::Refresh(void)
{
  _current_tag = 0;
}
