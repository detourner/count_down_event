#ifndef _LED_H_
#define _LED_H_
#include <Arduino.h>

class Led
{
    private :
        // Setup
        static const uint32_t _delay_between_two_cycle = 1000;
        static const uint32_t _delay_between_two_wink = 400;
        static const uint32_t _delay_wink = 100;

    public :
        Led() {};
        void    Setup(int led_pin);

        void    CyclTask(void);

        void    setWinks(int winks) { _winkNumber = winks; }
        int     getWinks(void) { return _winkNumber; }

    private :

        void            LedOn();
        void            LedOff();

        bool            _led_is_on;
        int             _led_pin;

        int             _winkNumber;
        int             _winkCurrent;
        unsigned long   _prevTime;
        uint32_t        _currentTime;
        uint32_t        _wakeup;

};
#endif
