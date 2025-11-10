#include "timeout.h"

void TimeOut::Setup(uint32_t timeOut, uint32_t maxTimeOut, CallbackType callback)

{
    _maxTimeOut = ((maxTimeOut / 1000) > 999) ? 999000 : maxTimeOut;
    _setTimeOutDuration = (timeOut > _maxTimeOut ) ? _maxTimeOut : timeOut;
    _callback = callback;
    _posPrev = timeOut / 1000;


    _setTimeOut = false;
}

void TimeOut::CyclTask(int rotPos)
{
    unsigned long time = millis();
    _currentTime += (time - _prevTime);
    _prevTime = time;

    //define timeout
    if(_setTimeOut)
    {
        //reset timer
        if(rotPos != _posPrev)
        {
            _currentTime = 0 ;
            _posPrev = rotPos;
            _setTimeOutDuration = rotPos * 1000;

            if(_callback != nullptr)
            {
                _callback(_posPrev);
            }
        }
        if(_currentTime > 5000)
        {
            _setTimeOut = false;
            _currentTime = 0;

            if(_callback != nullptr)
            {
                _callback(0);
            }
        }
    }
    else
    {
        if(rotPos != _posPrev)
        {
            _setTimeOut = true;
            _currentTime = 0 ;
            _posPrev = rotPos;
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