#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

#include <Arduino.h>


typedef void (*CallbackType)(uint32_t);

class TimeOut
{
    public :

        TimeOut() { };
        void Setup(uint32_t timeOut, uint32_t maxTimeOut, CallbackType callback = nullptr);
        void CyclTask(int rotPos);

        void newEvent(void) { _newEvent = true; }
        bool getDispStatus(void) { return _switchOn; }
        uint32_t Infinity(void) { return _maxTimeOut; }


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
        int           _posPrev;


        CallbackType _callback;


};

#endif