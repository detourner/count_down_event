#ifndef _ROTARY_CODER_H_
#define _ROTARY_CODER_H_

#include <Arduino.h>

class rotaryCoder {
  public :
    rotaryCoder() { }

    void init(const int pin_irq, const int pin_updw, const bool invert = false);

    void setMin(int min) { _min = min; }
    void setMax(int max) { _max = max; }
    void setVal(int val) 
    { 
      noInterrupts(); 
      _val = val; 
      interrupts(); 
    }

    int getMin(void) { return _min; }
    int getMax(void) { return _max; }
    int getVal(void) 
    { 
      noInterrupts();
      long val = _val;
      interrupts();
      return val; 
    }


  private:
    int _min;
    int _max;
    volatile int _val;

    int _pin_updw;
    int _invert;

    volatile long  _timeIRQ;

    void handler();

    // global interrupt service routine
    static void isrArg(void* arg) 
    {
      rotaryCoder* self = static_cast<rotaryCoder*>(arg);
      self->handler();
  }


};

#endif
