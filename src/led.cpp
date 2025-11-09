#include "led.h"

void Led::Setup(int led_pin)
{
    _led_pin = led_pin;
    pinMode(_led_pin, OUTPUT);

    LedOff();

    _winkCurrent = 0 ;
    _winkNumber = 0 ;
    _wakeup = 0;
    _currentTime = 0;
    _prevTime = 0 ;

}

void Led::CyclTask(void)
{
    unsigned long time = millis();
    _currentTime += (time - _prevTime);
    _prevTime = time;

    if(_currentTime > _wakeup)
    {
        _currentTime = 0;

        if(_led_is_on == true)
        {
            LedOff();
            _wakeup = (_winkCurrent > 0) ? _delay_between_two_wink :
                                           _delay_between_two_cycle;
        }
        else
        {
            _wakeup = _delay_wink;
            if(_winkCurrent > 0)
            {
                LedOn();
                _winkCurrent--;
            }
            else
            {
                _winkCurrent = _winkNumber;
            }
        }
    }
}

void Led::LedOff(void)
{
    _led_is_on = false;
    digitalWrite(_led_pin, LOW);
}

void Led::LedOn(void)
{
    _led_is_on = true;
    digitalWrite(_led_pin, HIGH);
}
