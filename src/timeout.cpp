#include "timeout.h"

void TimeOut::Setup(uint32_t timeOut, uint32_t maxTimeOut, CallbackTypeTimeOut callback)

{
    _maxTimeOut = ((maxTimeOut / 1000) > 999) ? 999000 : maxTimeOut;
    _setTimeOutDuration = (timeOut > _maxTimeOut ) ? _maxTimeOut : timeOut;
    _callback = callback;
    _posPrevTimeOut = timeOut / 1000;


    _setTimeOut = false;
}

void TimeOut::CyclTask(int rotPosTimeOut, int rotPosBrightness)
{
    unsigned long time = millis();
    if(time - _prevTime < 50) // update every 50ms
    {
        return;
    }
    _currentTime += (time - _prevTime);
    _prevTime = time;

    //define timeout
    if(_setTimeOut)
    {
        //reset timer
        if(rotPosTimeOut != _posPrevTimeOut)
        {
            _currentTime = 0 ;
            _posPrevTimeOut = rotPosTimeOut;
            _setTimeOutDuration = rotPosTimeOut * 1000;

            if(_callback != nullptr)
            {
                _callback(_posPrevTimeOut, TIMEOUT_MODE_TIMEOUT);
            }
        }
        if(_currentTime > 5000)
        {
            _setTimeOut = false;
            _currentTime = 0;

            if(_callback != nullptr)
            {
                _callback(_posPrevTimeOut, TIMEOUT_MODE_TIMEOUT_END);
            }
        }
    }
    else if (_setBrightness)
    {
        if(rotPosBrightness != _posPrevBrightness)
        {
            _currentTime = 0 ;
            _posPrevBrightness = rotPosBrightness;
            if(_callback != nullptr)
            {
                _callback(rotPosBrightness, TIMEOUT_MODE_BRIGHTNESS);
            }
        }
        if(_currentTime > 5000)
        {
            _setBrightness = false;
            _currentTime = 0;

            if(_callback != nullptr)
            {
                _callback(rotPosBrightness, TIMEOUT_MODE_BRIGHTNESS_END);
            }
        }
    }
    else
    {
        if(rotPosTimeOut != _posPrevTimeOut)
        {
            _setTimeOut = true;
            _currentTime = 0 ;
            _posPrevTimeOut = rotPosTimeOut;
        }
        else if(rotPosBrightness != _posPrevBrightness)
        {
            _setBrightness = true;
            _currentTime = 0 ;
            _posPrevBrightness = rotPosBrightness;
        }

        if(_newEvent)
        {
            _newEvent = false;
            _currentTime = 0;
            _switchOn = true;
        }

        if(_currentTime > _setTimeOutDuration)
        {
            _switchOn = false;
            _currentTime = _setTimeOutDuration + 50;

        }
        else
        {
            _switchOn = true;
        }
    }
}