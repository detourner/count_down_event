#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

#include <Arduino.h>

typedef enum
{
    TIMEOUT_MODE_TIMEOUT = 0,
    TIMEOUT_MODE_TIMEOUT_END = 1,
    TIMEOUT_MODE_BRIGHTNESS = 2,
    TIMEOUT_MODE_BRIGHTNESS_END = 3
} timeoutMode_t;

typedef void (*CallbackTypeTimeOut)(uint32_t, timeoutMode_t);

class TimeOut
{
    public :

        TimeOut() { };
        void Setup(uint32_t timeOut, uint32_t maxTimeOut, CallbackTypeTimeOut callback = nullptr);
        void CyclTask(int rotPosTimeOut, int rotPosBrightness);

        void newEvent(void) { _newEvent = true; }
        bool getDispStatus(void) { return _switchOn; }
        uint32_t Infinity(void) { return _maxTimeOut / 1000; }


        int getPosMin(void) { return 5; }
        int getPosMax(void) { return _maxTimeOut / 1000; }
        int getPosDef(void) { return _setTimeOutDuration / 1000; }


    private :

        bool _newEvent;
        bool _switchOn = true;

        unsigned long _prevTime;
        uint32_t      _currentTime;
        uint32_t      _setTimeOutDuration;
        uint32_t      _maxTimeOut;
        bool          _setTimeOut;
        bool          _setBrightness;
        int           _posPrevTimeOut;
        int           _posPrevBrightness;


        CallbackTypeTimeOut _callback;


};

#endif