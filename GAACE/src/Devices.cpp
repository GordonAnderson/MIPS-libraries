#include "Devices.h"

// Counts to value and value to count conversion functions.
// Overloaded for both DACchan and ADCchan structs.
float Counts2Value(int Counts, DACchan *dac)
{
  return (Counts - dac->b) / (dac->m);
}

float Counts2Value(int Counts, ADCchan *adc)
{
  return (Counts - adc->b) / (adc->m);
}

int Value2Counts(float Value, DACchan *dac)
{
  int counts;

  counts = (Value * dac->m) + dac->b;
  if (counts < 0) counts = 0;
  if (counts > 65535) counts = 65535;
  return (counts);
}

int Value2Counts(float Value, ADCchan *adc)
{
  int counts;

  counts = (Value * adc->m) + adc->b;
  if (counts < 0) counts = 0;
  if (counts > 65535) counts = 65535;
  return (counts);
}

float AnalogIn (int (*readadc)(int8_t chan), ADCchan *adc, int num)
{
	if(num <= 1) return Counts2Value(readadc(adc->Chan),adc);
	int value = 0;
	for(int i = 0;i<num;i++) value += readadc(adc->Chan);
	return(value/num);
}

float Filter(float lastV, float newV, float filter)
{
   return lastV * (1 - filter) + newV * filter;
}

void AnalogOut(void (*writedac)(int8_t chan, int counts), DACchan *dac, float value)
{
	writedac(dac->Chan,Value2Counts(value,dac));
}

// AD5592 IO routines. This is a analog and digitial IO chip with
// a SPI interface. The following are low level read and write functions,
// the modules using this device are responsible for initalizing the chip.

// Write to AD5592
void AD5592write(int CS, uint8_t reg, uint16_t val)
{
  int iStat;

  digitalWrite(CS,LOW);
  SPI.transfer(((reg << 3) & 0x78) | (val >> 8));
  SPI.transfer(val & 0xFF);
  digitalWrite(CS,HIGH);
}

// Read from AD5593R
// returns 16 bit value read
int AD5592readWord(int CS)
{
  uint16_t  val;

  digitalWrite(CS,LOW);
  val = SPI.transfer16(0);
  digitalWrite(CS,HIGH);
  return val;
}

// Returns -1 on error. Error is flaged if the readback channel does not match the
// requested channel.
// chan is 0 thru 7
int AD5592readADC(int CS, int8_t chan)
{
   uint16_t  val;

   // Write the channel to convert register
   AD5592write(CS, 2, 1 << chan);
   delayMicroseconds(1);	// Conversion happens here
   // Dummy read
   digitalWrite(CS,LOW);
   SPI.transfer16(0);
   digitalWrite(CS,HIGH);
   // Read the ADC data 
   digitalWrite(CS,LOW);
   val = SPI.transfer16(0);
   digitalWrite(CS,HIGH);
   // Test the returned channel number
   if(((val >> 12) & 0x7) != chan) return(-1);
   // Left justify the value and return
   val <<= 4;
   return(val & 0xFFF0);
}

int AD5592readADC(int CS, int8_t chan, int8_t num)
{
  int i,j, val = 0;

  for (i = 0; i < num; i++) 
  {
    j = AD5592readADC(CS, chan);
    if(j == -1) return(-1);
    val += j;
  }
  return (val / num);
}

// There is a potential for this function to be interrupted and then called again
// from the interrupt. The code was updated 12/15/23 to queue up a request when
// busy and execute when finished.
void AD5592writeDAC(int CS, int8_t chan, int val)
{
   uint16_t      d;
   static bool   busy=false;
   static bool   queded = false;
   static int    qCS,qval;
   static int8_t qchan;
   
   if(busy)
   {
    queded = true;
    qCS=CS;
    qval=val;
    qchan=chan;
    return;
   }
   busy = true;
   // convert 16 bit DAC value into the DAC data data reg format
   d = ((val>>4) & 0x0FFF) | (((uint16_t)chan) << 12) | 0x8000;
   digitalWrite(CS,LOW);
   val = SPI.transfer((uint8_t)(d >> 8));
   val = SPI.transfer((uint8_t)d);
   digitalWrite(CS,HIGH);
   busy = false;
   if(queded)
   {
    queded = false;
    AD5592writeDAC(qCS,qchan,qval);
   }
}

// End of AD5592 routines