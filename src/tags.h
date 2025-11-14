#ifndef _TAGS_H_
#define _TAGS_H_

#include <Arduino.h>
#include "MFRC522.h"


#define TAGS_NUM_OF_RETRY (3)
// RFID pinout
#define SS_PIN      5
#define RST_PIN     4

typedef void (*CallbackTypeTag)(uint32_t);

class tags
{
    public :

        tags() { };

        // Setup, must be call in setup() function in main file
        void Setup(CallbackTypeTag callback = nullptr);

        // Task cycl, muste be call in loop() function in main file
        // as soon as possible.
        // The reading tag operation is performed every 30ms!
        void CyclTask(void);

        void Refresh(void);

        // Return true if a tag is present
        bool     GetTagIsPresent(void ) { return _tagIsPresent; }

        // Return Id tag. If tag is not present, return last one id.
        uint32_t GetTagId(void) { return _current_tag; }

    private :

      bool          _tagIsPresent;        // True if a tag is reading...
      uint32_t      _current_tag;         // Tag id (if tag is not present, send last id)

      int           _retry;               // counter of retry in case of default duraing read tags.

      unsigned long _prevTime = 0;
      uint8_t       _currentTime = 0;

      CallbackTypeTag _callback;

};

#endif
