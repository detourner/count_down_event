#include "rotary_coder.h"


void rotaryCoder::init(const int pin_irq, const int pin_updw, const bool invert)
{
    _pin_updw = pin_updw;
    _invert = invert;
    _max = 100;
    _min = 0;
    _val = 0;
    _timeIRQ = 0;
    pinMode(pin_updw, INPUT);
    pinMode(pin_irq, INPUT);
    attachInterruptArg(pin_irq, &rotaryCoder::isrArg, this, FALLING);
}


void rotaryCoder::handler()
{
    static TickType_t lastTick = 0;
    TickType_t now = xTaskGetTickCountFromISR();
    if (now - lastTick < pdMS_TO_TICKS(10)) return; // debounce 10ms
    lastTick = now;

    bool in = digitalRead(_pin_updw);
    in = _invert ? !in : in;
    if(in) // Test is go up or go down...
    {
        if(_val < _max)
        {
            _val++;
        }
    }
    else
    {
        if(_val > _min)
        {
            _val--;
        }

    }
}
