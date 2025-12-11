#include "nixies_manager.h"
#include "preferences_manager.h"

void NixiesManager::Setup(uint32_t timeOut, uint32_t maxTimeOut, int brightness, CallbackTypeNixiesManager callback)

{
    _maxTimeOut = maxTimeOut * 1000; // convert to ms
    _setTimeOutDuration = ( (timeOut * 1000) > _maxTimeOut ) ? _maxTimeOut : (timeOut * 1000);
    
    _callback = callback;
    _posPrevTimeOut = timeOut;

    _posPrevBrightness = brightness;    

    _setTimeOut = false;
    _setBrightness = false;
}

void NixiesManager::CyclTask(int rotPosTimeOut, int rotPosBrightness)
{
    unsigned long time = millis();
    if(time - _prevTime < 50) // update every 50ms
    {
        return;
    }
    _currentTime += (time - _prevTime);
    _prevTime = time;

    
    if(_setTimeOut) //define timeout in progress
    {
        if(rotPosTimeOut != _posPrevTimeOut) // rotary encoder has been moved
        {
            _currentTime = 0 ;
            _posPrevTimeOut = rotPosTimeOut;
            
            if(_callback != nullptr)
            {
                _callback(_posPrevTimeOut, TIMEOUT_MODE_TIMEOUT);
            }
        }
        if(_currentTime > 5000 || _newEvent) // rotary encode has not moved since 5sec or new event
        {
            // end of define timeout sequence
            _setTimeOut = false;
            _currentTime = 0;
            _setTimeOutDuration = rotPosTimeOut * 1000;

            if(_callback != nullptr)
            {
                _callback(_posPrevTimeOut, TIMEOUT_MODE_TIMEOUT_END);
            }
        }
    }
    else if (_setBrightness) // define brightness in progress
    {
        if(rotPosBrightness != _posPrevBrightness) // rotary encoder has been moved
        {
            _currentTime = 0 ;
            _posPrevBrightness = rotPosBrightness;
            if(_callback != nullptr)
            {
                _callback(rotPosBrightness, TIMEOUT_MODE_BRIGHTNESS);
            }
        }
        if(_currentTime > 5000 || _newEvent) // rotary encode has not moved since 5sec or new event
        {
            // end of define brightness sequence
            _setBrightness = false;
            _currentTime = 0;
            
            if(_callback != nullptr)
            {
                _callback(rotPosBrightness, TIMEOUT_MODE_BRIGHTNESS_END);
            }
        }
    }
    else // idle 
    {
        if(rotPosTimeOut != _posPrevTimeOut)
        {
            // start define timeout sequence 
            _setTimeOut = true;
            _currentTime = 0 ;
            _posPrevTimeOut = rotPosTimeOut;
        }
        else if(rotPosBrightness != _posPrevBrightness)
        {
           // start define brightness sequence
            _setBrightness = true;
            _currentTime = 0 ;
            _posPrevBrightness = rotPosBrightness;
        }

        if(_currentTime > _setTimeOutDuration && _switchOn == true)
        {
            // is time to turn off display
            _switchOn = false;
            _currentTime = _setTimeOutDuration + 50;

            if(_callback != nullptr)
            {
                _callback(0, TIEMOUT_IS_ACTIVE);
            }
        }
    }

    if(_newEvent)
    {
        // a new event required to update display and abord sequences (brightness or timeout)
        _newEvent = false;
        _currentTime = 0;
        _switchOn = true;
        _setBrightness = false;
        _setTimeOut = false;
    }
}