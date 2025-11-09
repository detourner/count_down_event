#include <nixies_drv.h>

NixiesDriver::NixiesDriver(void)
{
   _prevTime = 0;
   _currentTime = 0;
   _durationOn = 0;
   _durationTotal = 0;
   _currentValueDisp = 1000;
}

void NixiesDriver::Setup()
{
    // Set all nixies output !
    pinMode( NIXIES_DRV_POL,   OUTPUT);
    pinMode( NIXIES_DRV_BL,    OUTPUT);
    pinMode( NIXIES_DRV_LE,    OUTPUT);
    pinMode( NIXIES_DRV_DATA,  OUTPUT);
    pinMode( NIXIES_DRV_CLK,   OUTPUT);


    digitalWrite( NIXIES_DRV_POL,   LOW);
    digitalWrite( NIXIES_DRV_BL,    HIGH);
    digitalWrite( NIXIES_DRV_LE,    LOW);
    digitalWrite( NIXIES_DRV_DATA,  LOW);
    digitalWrite( NIXIES_DRV_CLK,   LOW);


    // Configurer le canal PWM
    ledcSetup(NIXIES_PWM_CHANNEL, NIXIES_PWM_FREQUENCY, NIXIES_PWM_RESOLUTION);
  
    // Attacher le GPIO au canal PWM
    ledcAttachPin(NIXIES_DRV_BL, NIXIES_PWM_CHANNEL);

    // write inverted value: 0 => max, max => 0
    ledcWrite(NIXIES_PWM_CHANNEL, GetMaxBrightness()); // set off (inverted)

}

void NixiesDriver::CyclTask(void)
{
    unsigned long time = millis();
    _currentTime += (time - _prevTime);
    _prevTime = time;

    if(_durationOn == 0) // Duty cycle is 0%
    {
        // Allways Off;
        // Off means write max (inverted)
        ledcWrite(NIXIES_PWM_CHANNEL, GetMaxBrightness());
        _currentTime = 0;
    }
    else if(_durationTotal == _durationOn) // Duty cycle is 100%
    {
        // Allways On !
        // On means write inverted brightness
        ledcWrite(NIXIES_PWM_CHANNEL, GetMaxBrightness() - _brightness);
        _currentTime = 0;
    }
    else if(_currentTime < _durationOn)
    {
        // is On !
       ledcWrite(NIXIES_PWM_CHANNEL, GetMaxBrightness() - _brightness);
    }
    else if(_currentTime <_durationTotal)
    {
        // is Off !
        ledcWrite(NIXIES_PWM_CHANNEL, GetMaxBrightness());
    }
    else
    {
      _currentTime = 0;
    }
}

void NixiesDriver::SetBrightness(const uint16_t value)
{
    _brightness = (value < GetMaxBrightness()) ? value :  GetMaxBrightness();
}

void NixiesDriver::SetBlink(const uint32_t periodeMs, const uint32_t dutyCycle)
{
    if(_periodeMsprev == periodeMs && _dutyCycleprev == dutyCycle )
    {
        return;
    }
    _periodeMsprev = periodeMs;
    _dutyCycleprev = dutyCycle;

    if(dutyCycle < 100)
    {
        _durationOn = (periodeMs * dutyCycle) / 100 ;
    }
    else
    {
        _durationOn = periodeMs;
    }

    _durationTotal = periodeMs;
    _currentTime = 0; // Reset the current time (avoid any bugs !)
}

void NixiesDriver::DispValue(const uint16_t value)
{
    if(_currentValueDisp == value)
    {
      return; // nothing to change !
    }
    const uint16_t v = (value < 1000)  ? value : 999;
    String s = String(v, DEC);

    const uint8_t sl = s.length();
    const uint8_t digit1 = (sl >= 3) ? (char)(s[sl - 3]) - '0' : 31;
    const uint8_t digit2 = (sl >= 2) ? (char)(s[sl - 2]) - '0' : 21;
    const uint8_t digit3 = (sl >= 1) ? (char)(s[sl - 1]) - '0' : 11;


    const uint32_t encode = (1 << (digit1 + 0  ) )  |
                            (1 << (digit2 + 10 ) )  |
                            (1 << (digit3 + 20 ) );

    LoadShiftRegister(encode);

    _currentValueDisp = value;
}

void NixiesDriver::LoadShiftRegister(const uint32_t value)
{
    // Note: Is it not possible to use SPI, because the minimum clock frequency
    // is 100kHz. This value is too high to the logicial shift level ULN2803A.
    // This method used a simple shift register...
    uint32_t val = value;

    digitalWrite( NIXIES_DRV_LE,   HIGH);
    // for each of 32 bits...
    for(uint8_t i=0; i<32; i++)
    {
        digitalWrite( NIXIES_DRV_CLK,   LOW);
        if( val & ((uint32_t)0x80000000) )
            digitalWrite( NIXIES_DRV_DATA,   LOW);
        else
            digitalWrite( NIXIES_DRV_DATA,   HIGH);
        val <<= 1;
        delayMicroseconds(20); // Slowly, ULN2803A switch in 1us !
        digitalWrite( NIXIES_DRV_CLK,   HIGH);
        delayMicroseconds(30);
    }
    delayMicroseconds(30);
    // Load latches !
    digitalWrite( NIXIES_DRV_LE,   LOW);
    delayMicroseconds(30);
    digitalWrite( NIXIES_DRV_LE,   HIGH);
}
