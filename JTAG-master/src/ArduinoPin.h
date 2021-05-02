#ifndef ARDUINOPIN_H
#define ARDUINOPIN_H

#include <inttypes.h>
#include <Arduino.h>

#ifdef __SAM3X8E__
	#define GPIO_PORT_TYPE Pio*
	#define GPIO_BIT_TYPE uint32_t
#else
#ifdef ARDUINO_ARCH_SAMD
	#define GPIO_PORT_TYPE PortGroup *
	#define GPIO_BIT_TYPE uint32_t
#else
    #define GPIO_PORT_TYPE uint8_t
    #define GPIO_BIT_TYPE uint8_t
#endif
#endif

class ArduinoPin
{
private:
	GPIO_BIT_TYPE m_bit;
	GPIO_PORT_TYPE m_port;

	void setBit(GPIO_BIT_TYPE n) { m_bit = n; }
	void setPort(GPIO_PORT_TYPE n) { m_port = n; }

	uint8_t m_pin;
	uint8_t m_mode;

protected:
	GPIO_BIT_TYPE getBit() const { return m_bit; }
	GPIO_PORT_TYPE port() const { return m_port; }

public:
	ArduinoPin(uint8_t pin, uint8_t mode);
	~ArduinoPin() {}

	uint8_t pin() const { return m_pin; }

	uint8_t mode() const { return m_mode; }
};

#ifdef ARDUINO_ARCH_AVR
	#define GPIO_REGISTER_TYPE uint8_t
#elif defined ARDUINO_ARCH_ESP8266
	#define GPIO_REGISTER_TYPE uint32_t
#elif defined ARDUINO_ARCH_ESP32
	#define GPIO_REGISTER_TYPE uint32_t
#elif defined ARDUINO_ARCH_SAMD
	#define GPIO_REGISTER_TYPE uint32_t
#elif defined __SAM3X8E__
	#define GPIO_REGISTER_TYPE uint32_t
#else
	#define GPIO_REGISTER_TYPE uint8_t
#endif

class ArduinoInputPin : public ArduinoPin
{
private:
	volatile GPIO_REGISTER_TYPE *m_in;

	volatile GPIO_REGISTER_TYPE *in() const { return m_in; }

public:
	ArduinoInputPin(uint8_t pin);
	~ArduinoInputPin() {}

//	uint8_t read() const { return *in() & getBit(); }
   uint8_t read() const { if(*in() & getBit()) return true; else return false; }
};


class ArduinoOutputPin : public ArduinoPin
{
private:
	volatile GPIO_REGISTER_TYPE *m_out;

	volatile GPIO_REGISTER_TYPE *out() const { return m_out; }

public:
	ArduinoOutputPin(uint8_t pin);
	~ArduinoOutputPin() {}

	void clr() const { *out() &= ~getBit(); }
	void set() const { *out() |= getBit(); }
};

#endif // ARDUINOPIN_H
