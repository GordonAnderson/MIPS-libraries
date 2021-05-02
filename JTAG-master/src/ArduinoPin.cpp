#include "ArduinoPin.h"

#include <Arduino.h>

GPIO_BIT_TYPE mydigitalPinToBitMask(uint8_t pin) __attribute__((weak));
GPIO_PORT_TYPE mydigitalPinToPort(uint8_t pin) __attribute__((weak));
void mypinMode(GPIO_BIT_TYPE p, uint8_t m) __attribute__((weak));

GPIO_BIT_TYPE mydigitalPinToBitMask(uint8_t pin)
{
   digitalPinToBitMask(pin);
}

GPIO_PORT_TYPE mydigitalPinToPort(uint8_t pin)
{
   digitalPinToPort(pin);
}

void mypinMode(GPIO_BIT_TYPE p, uint8_t m)
{
   pinMode(p,m);
}

ArduinoPin::ArduinoPin(uint8_t pin, uint8_t mode)
: m_bit(0)
, m_port(0)
, m_pin(pin)
, m_mode(mode)
{
	setBit(mydigitalPinToBitMask(pin));
	setPort(mydigitalPinToPort(pin));
	mypinMode(ArduinoPin::pin(), ArduinoPin::mode());
}

ArduinoInputPin::ArduinoInputPin(uint8_t pin)
: ArduinoPin(pin, INPUT_PULLUP)
, m_in(portInputRegister(port()))
{
}

ArduinoOutputPin::ArduinoOutputPin(uint8_t pin)
: ArduinoPin(pin, OUTPUT)
, m_out(portOutputRegister(port()))
{
}

