#ifndef Devices_h
#define Devices_h
#include <arduino.h>
#include <SPI.h>

enum Devices
{
  ADC_,
  PWM,
  AD5592  
};

typedef struct
{
  int8_t  Chan;         // ADC channel number 0 through max channels for chip
  float   m;            // Calibration parameters to convert channel to engineering units
  float   b;            // ADCcounts = m * value + b, value = (ADCcounts - b) / m
} ADCchan;

typedef struct
{
  int8_t  Chan;         // DAC channel number 0 through max channels for chip
  float   m;            // Calibration parameters to convert engineering to DAC counts
  float   b;            // DACcounts = m * value + b, value = (DACcounts - b) / m
} DACchan;

float AnalogIn (int (*readadc)(int8_t chan), ADCchan *adc, int num = 1);
void  AnalogOut(void (*writedac)(int8_t chan, int counts), DACchan *dac, float value);

float Filter(float lastV, float newV, float filter = 0.1);

float Counts2Value(int Counts, DACchan *dac);
float Counts2Value(int Counts, ADCchan *adc);
int   Value2Counts(float Value, DACchan *dac);
int   Value2Counts(float Value, ADCchan *adc);

void AD5592write(int CS, uint8_t reg, uint16_t val);
int  AD5592readWord(int CS);
int  AD5592readADC(int CS, int8_t chan);
int  AD5592readADC(int CS, int8_t chan, int8_t num);
void AD5592writeDAC(int CS, int8_t chan, int val);

#endif
