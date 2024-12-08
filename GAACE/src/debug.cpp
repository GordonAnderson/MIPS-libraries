#include "debug.h"

static commandProcessor *cp;
static debug *db;

static uint32_t  baseAddress = 0;
static uint32_t  addOffset = 0;

static void (*debugFunction)(void) = NULL;

static void callDebug(void)
{
  if(debugFunction != NULL) debugFunction();
}

static void setADCres(void)
{
  int i;

  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&i,2,16))
  {
    analogReadResolution(i);
    cp->sendACK();
    return;
  }
  cp->sendNAK();
}

static void setDACres(void)
{
  int i;

  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&i,2,16))
  {
    analogWriteResolution(i);
    cp->sendACK();
    return;
  }
  cp->sendNAK();
}
static void getADC(void)
{
  int i;

  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&i,0,100))
  {
    cp->sendACK(false);
    cp->println(analogRead(i));
    return;
  }
  cp->sendNAK();
}
static void setDAC(void)
{
  int ch,i;

  if(cp->getNumArgs() != 2) {cp->sendNAK(); return;} 
  if(cp->getValue(&ch,0,100))
  {
    if(cp->getValue(&i,0,65535))
    {
      analogWrite(ch,i);
      cp->sendACK();
      return;
    }
  }
  cp->sendNAK();
}

#if SAMD21_SERIES
static __inline__ void syncADC() __attribute__((always_inline, unused));
static void syncADC() 
{
  while (ADC->STATUS.bit.SYNCBUSY == 1);
}
static int readADC(int ch)
{
  syncADC();
  ADC->INPUTCTRL.bit.MUXPOS = ch;
  syncADC();
  ADC->CTRLA.bit.ENABLE = 0x01;             // Enable AD
  // Start conversion
  syncADC();
  ADC->SWTRIG.bit.START = 1;
  // Waiting for the 1st conversion to complete
  while (ADC->INTFLAG.bit.RESRDY == 0);
  // Clear the Data Ready flag
  ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
  // Start conversion again, since The first conversion after the reference is changed must not be used.
  syncADC();
  ADC->SWTRIG.bit.START = 1;
  // Store the value
  while (ADC->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
  int valueRead = ADC->RESULT.reg;
  syncADC();
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  syncADC();
  return valueRead;
}
#endif

#if SAMD51_SERIES
int readADC(Adc *adc,int ch)
{
  while( adc->SYNCBUSY.reg & ADC_SYNCBUSY_INPUTCTRL ); //wait for sync
  adc->INPUTCTRL.bit.MUXPOS = ch;
  while( adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
  adc->CTRLA.bit.ENABLE = 0x01;             // Enable ADC
  // Start conversion
  while( adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
  adc->SWTRIG.bit.START = 1;
  // Clear the Data Ready flag
  adc->INTFLAG.reg = ADC_INTFLAG_RESRDY;
  // Start conversion again, since The first conversion after the reference is changed must not be used.
  adc->SWTRIG.bit.START = 1;
  // Store the value
  while (adc->INTFLAG.bit.RESRDY == 0);   // Waiting for conversion to complete
  int valueRead = adc->RESULT.reg;
  while( adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
  adc->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  while( adc->SYNCBUSY.reg & ADC_SYNCBUSY_ENABLE ); //wait for sync
  return valueRead;
}
#endif

static void cpuTemp(void)
{
  float treal=-1001;

  if(cp->getNumArgs() != 0) {cp->sendNAK(); return;} 
#if SAMD21_SERIES
  uint32_t  *tempLog = (uint32_t *)0x00806030;
  float     roomTemp,hotTemp;
  float     room1V,hot1V;
  uint16_t  roomADC,hotADC;

  roomTemp = (float)(tempLog[0] & 0xFF) + (float)((tempLog[0] >> 8) & 0x0F) / 10.0;
  hotTemp = (float)((tempLog[0] >> 12) & 0xFF) + (float)((tempLog[0] >> 20) & 0x0F) / 10.0;
  room1V = 1.0 - (float)((int8_t)((tempLog[0] >> 24) & 0xFF))/1000.0;
  hot1V  = 1.0 - (float)((int8_t)((tempLog[1] & 0xFF)))/1000.0;
  roomADC = (tempLog[1] >> 8) & 0xFFF;
  hotADC = (tempLog[1] >> 20) & 0xFFF;
  int gain = ADC->INPUTCTRL.bit.GAIN;
  int ref = ADC->REFCTRL.bit.REFSEL;
  analogReference(AR_INTERNAL1V0);
  SYSCTRL->VREF.reg |= SYSCTRL_VREF_TSEN;
  int valueRead = readADC(0x18);
  if(ADC->CTRLB.bit.RESSEL == ADC_CTRLB_RESSEL_10BIT_Val) valueRead = valueRead << 2;
  if(ADC->CTRLB.bit.RESSEL == ADC_CTRLB_RESSEL_8BIT_Val) valueRead = valueRead << 4;
  ADC->INPUTCTRL.bit.GAIN = gain;
  ADC->REFCTRL.bit.REFSEL = ref;
  treal = roomTemp+((((float)valueRead)/4095.0) - (((float)roomADC)*room1V/4095.0)) * (hotTemp - roomTemp)/((((float)hotADC) * hot1V/4095.0) - (((float)roomADC) * room1V/4095.0));
#endif
#if SAMD51_SERIES
  uint32_t  *tempLog = (uint32_t *)0x00800100;
  float   TL,TH;
  int16_t VPL,VPH,VCL,VCH;
  TL = (float)(tempLog[0] & 0xFF) + (float)((tempLog[0] >> 8) & 0x0F) / 10.0;
  TH = (float)((tempLog[0] >> 12) & 0xFF) + (float)((tempLog[0] >> 20) & 0x0F) / 10.0;
  VPL = (tempLog[1] >> 8) & 0xFFF;
  VPH = (tempLog[1] >> 20) & 0xFFF;
  VCL = tempLog[2] & 0xFFF;
  VCH = (tempLog[2] >> 12) & 0xFFF;

  int ondemand = SUPC->VREF.bit.ONDEMAND;
  int vrefoe   = SUPC->VREF.bit.VREFOE;
  SUPC->VREF.bit.ONDEMAND = 0;
  SUPC->VREF.bit.VREFOE = 0;
	SUPC->VREF.bit.TSEN = 1;
	SUPC->VREF.bit.TSSEL = 0;  // 0 = PTAT, 1 = CTAT
  Adc *adc = ADC0;
  // PTAT = 0x1C, CTAT = 0x1D
	SUPC->VREF.bit.TSSEL = 0;  // 0 = PTAT, 1 = CTAT
  int16_t TP = readADC(adc,0x1C);
  if(adc->CTRLB.bit.RESSEL == ADC_CTRLB_RESSEL_10BIT_Val) TP = TP << 2;
  if(adc->CTRLB.bit.RESSEL == ADC_CTRLB_RESSEL_8BIT_Val) TP = TP << 4;
	SUPC->VREF.bit.TSSEL = 1;  // 0 = PTAT, 1 = CTAT
  int16_t TC = readADC(adc,0x1D);
  if(adc->CTRLB.bit.RESSEL == ADC_CTRLB_RESSEL_10BIT_Val) TC = TC << 2;
  if(adc->CTRLB.bit.RESSEL == ADC_CTRLB_RESSEL_8BIT_Val) TC = TC << 4;
  treal = (TL*VPH*TC - VPL*TH*TC - TL*VCH*TP + TH*VCL*TP)/(VCL*TP - VCH*TP - VPL*TC + VPH*TC);
  SUPC->VREF.bit.ONDEMAND = ondemand;
  SUPC->VREF.bit.VREFOE = vrefoe;
	SUPC->VREF.bit.TSEN = 0;
#endif
#if SAM3X8
  /* Enable ADC channel 15 and turn on temperature sensor */
  ADC->ADC_CHER = 1 << 15;
  ADC->ADC_ACR |= ADC_ACR_TSON;
  /* Start conversion. */
  ADC->ADC_CR = ADC_CR_START;
  /* Wait for end of the conversion. */
  while (ADC->ADC_ISR & ADC_ISR_EOC15 == ADC_ISR_EOC15);
  delay(100); // Keep this delay      
  /* Read the value. */ 
  int mV = ADC->ADC_LCDR;
  /* Start conversion. */
  ADC->ADC_CR = ADC_CR_START;
  /* Wait for end of the conversion. */
  while (ADC->ADC_ISR & ADC_ISR_EOC15 == ADC_ISR_EOC15);
  delay(100); // Keep this delay      
  /* Read the value. */ 
  mV = ADC->ADC_LCDR;
  /* Disable channel 15. */
  ADC->ADC_CHDR = 1 << 15; 
  treal = (( (3300 * mV)/4096 ) - 800) * 0.37736 + 25.5;
#endif
#ifdef ARDUINO_TEENSY40
	treal = tempmonGetTemp();
#endif
  if(treal < -1000) {cp->sendNAK(); return;} 
  cp->sendACK(false);
  cp->println(treal);
}

static void UUID(void)
{
   unsigned int adwUniqueID[4]; 
   unsigned int *val=NULL;

  if(cp->getNumArgs() != 0) {cp->sendNAK(); return;} 
#if SAMD21_SERIES
  val = (unsigned int *)0x0080A00C;
#endif
#if SAMD51_SERIES
  val = (unsigned int *)0x008061FC;
#endif
#if SAM3X8
  _EEFC_ReadUniqueID(adwUniqueID);
  val = adwUniqueID;
#endif
  if(val == NULL) {cp->sendNAK(); return;} 
  cp->sendACK(false);
  cp->print("ID: ");
  for (byte b = 0 ; b < 4 ; b++) cp->print ((uint32_t) val[b], HEX);
  cp->print();
}

void debug::softwareReset(void)
{
  if(cp->getNumArgs() != 0) {cp->sendNAK(); return;} 
  cp->sendACK();
#if SAMD21_SERIES
  NVIC_SystemReset();  
#endif
#if SAMD51_SERIES
  NVIC_SystemReset();  
#endif
#if SAM3X8
  const int RSTC_KEY = 0xA5;
  RSTC->RSTC_CR = RSTC_CR_KEY(RSTC_KEY) | RSTC_CR_PROCRST | RSTC_CR_PERRST;
#endif
#ifdef ARDUINO_TEENSY40
  // in globals declaration section
  #define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
  #define CPU_RESTART_VAL 0x5FA0004
  #define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

  CPU_RESTART;
#endif
  while(true);
}

static void upTime(void)
{
  if(cp->getNumArgs() != 0) {cp->sendNAK(); return;} 
  cp->sendACK(false);
  cp->print("System has been up for at least: ");
  float uptime = (float)millis() / 60000;
  cp->print(uptime);
  cp->println(" minutes");
}

static void  setAddress(void)
{
  uint32_t  i;

  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&i,0,0,HEX))
  {
    cp->sendACK(false);
    baseAddress = i;
    cp->print("Base address,hex = ");
    cp->println(baseAddress,HEX);
    addOffset = 0;
 }
  else cp->sendNAK();
}

static void  setOffset(void)
{
  uint32_t  i;

  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&i,0,0,HEX))
  {
    cp->sendACK(false);
    addOffset = i;
    cp->print("Address offset,hex = ");
    cp->println(addOffset,HEX);
    addOffset = 0;
 }
  else cp->sendNAK();
}

static void writeMemory(void)
{
  char      *res;
  void      *ptr;
  uint32_t  uval;
  int       i;
  float     fval;
  bool      status;

  ptr = (void *)(baseAddress + addOffset);
  if(cp->getNumArgs() != 2) {cp->sendNAK(); return;} 
  if(cp->getValue(&res))
  {
     if(strcasecmp(res,"BYTE")==0)       {if((status=cp->getValue(&uval,0,0xFF,HEX))) *(uint8_t *)ptr = (uint8_t)uval;}
     else if(strcasecmp(res,"WORD")==0)  {if((status=cp->getValue(&uval,0,0xFFFF,HEX))) *(uint16_t *)ptr = (uint16_t)uval;}
     else if(strcasecmp(res,"DWORD")==0) {if((status=cp->getValue(&uval,0,0xFFFFFFFF,HEX))) *(uint32_t *)ptr = (uint32_t)uval;}
     else if(strcasecmp(res,"INT")==0)   {if((status=cp->getValue(&i))) *(int *)ptr = i;}
     else if(strcasecmp(res,"FLOAT")==0) {if((status=cp->getValue(&fval))) *(float *)ptr = fval;}
     cp->sendACK();
     return;
  }
  cp->sendNAK();
}

static void readMemory(void)
{
  char      *res;
  void      *ptr;

  ptr = (void *)(baseAddress + addOffset);
  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&res))
  {
    cp->sendACK(false);
     if(strcasecmp(res,"BYTE")==0) cp->println((int)*(uint8_t *)ptr,HEX);
     else if(strcasecmp(res,"WORD")==0) cp->println((int)*(uint16_t *)ptr,HEX);
     else if(strcasecmp(res,"DWORD")==0) cp->println((uint32_t)*(uint32_t *)ptr,HEX);
     else if(strcasecmp(res,"INT")==0) cp->println((int)*(int *)ptr);
     else if(strcasecmp(res,"FLOAT")==0) cp->println((float)*(float *)ptr);
     return;
  }
  cp->sendNAK();
}

#ifdef ESP_PLATFORM
static void freeRam(void) 
{
    cp->println(esp_get_minimum_free_heap_size());
}
#else
extern "C" char* sbrk(int incr);
extern char *__brkval;

static void freeRam(void) 
{
  if(cp->getNumArgs() != 0) {cp->sendNAK(); return;} 
  cp->sendACK(false);
  char top;
  int i = &top - reinterpret_cast<char*>(sbrk(0));
  cp->println(i);
}
#endif

static void dumpMemory(void)
{
  char sbuf[10];
  uint8_t *buf = (uint8_t *)(baseAddress + addOffset);

  if(cp->getNumArgs() != 0) {cp->sendNAK(); return;} 
  cp->sendACK(false);
  cp->print("Memory dump at: ");
  cp->println(baseAddress + addOffset,16);
  cp->print("     ");
  for(int j=0;j<16;j++)
  {
    sprintf(sbuf,"%02X ",j);
    cp->print(sbuf);
  }
  cp->print();
  for(int i=0;i<32;i++)
  {
    sprintf(sbuf,"%03X: ",i * 16);
    cp->print(sbuf);
    for(int j=0;j<16;j++)
    {
      sprintf(sbuf,"%02X ",buf[j + (i * 16)]);
      cp->print(sbuf);
    }
    cp->print();
  }
}

static void setPinMode(void)
{
  int  pin;
  char *mode;

  if(cp->getNumArgs() != 2) {cp->sendNAK(); return;} 
  if(cp->getValue(&pin, 0,100))
  {
    if(cp->getValue(&mode,"INPUT,OUTPUT,PULLUP"))
    {
      cp->sendACK();
      if(strcasecmp(mode,"INPUT")) pinMode(pin, INPUT);
      else if(strcasecmp(mode,"PULLUP")) pinMode(pin, INPUT_PULLUP);
      else if(strcasecmp(mode,"OUTPUT")) pinMode(pin, OUTPUT);
    }
    return;
  }
  cp->sendNAK();
}

static void setPin(void)
{
  int  pin;
  char *mode;

  if(cp->getNumArgs() != 2) {cp->sendNAK(); return;} 
  if(cp->getValue(&pin, 0,100))
  {
    if(cp->getValue(&mode,"LOW,HIGH,PULSE,SPLUSE"))
    {
      cp->sendACK();
      if(strcasecmp(mode,"LOW")) digitalWrite(pin,LOW);
      else if(strcasecmp(mode,"HIGH")) digitalWrite(pin,HIGH);
      else if(strcasecmp(mode,"PULSE"))
      {
        if(digitalRead(pin) == LOW) {digitalWrite(pin,HIGH); digitalWrite(pin,LOW);}
        else {digitalWrite(pin,LOW); digitalWrite(pin,HIGH);}
      }
      else if(strcasecmp(mode,"SPULSE"))
      {
        if(digitalRead(pin) == LOW) {digitalWrite(pin,HIGH); delay(1); digitalWrite(pin,LOW);}
        else {digitalWrite(pin,LOW); delay(1); digitalWrite(pin,HIGH);}
      }
      return;
    }
  }
  cp->sendNAK();
}

static void getPin(void)
{
  int pin;

  if(cp->getNumArgs() != 1) {cp->sendNAK(); return;} 
  if(cp->getValue(&pin, 0,100))
  {
    cp->sendACK(false);
    if(digitalRead(pin) == HIGH) cp->println("HIGH");
    else cp->println("LOW");
    return;
  }
  cp->sendNAK();
}

// This to add to this generic capability
// - Add esp32 processor support
// - Counter with threshold and output trigger generation
// - Frequency generation with burst
// - Add Watchdog timer support
//    - Support watchdog reset test function
static Command debugCmds[] =
{
  // Memory access commands
  {"SETADDRESS",CMDfunction,-1,(void *)setAddress,NULL,   "Set base address in hex"},
  {"SETOFFSET", CMDfunction,-1,(void *)setOffset,NULL,    "Set offset from base address in hex"},
  {"WRITE",     CMDfunction,-1,(void *)writeMemory,NULL,  "Write, BYTE,WORD,DWORD,INT, or FLOAT to baseaddress + offset"},
  {"READ",      CMDfunction,-1,(void *)readMemory,NULL,   "Read, BYTE,WORD,DWORD,INT, or FLOAT from baseaddress + offset"},
  {"DUMP",      CMDfunction,-1,(void *)dumpMemory,NULL,   "Dump memory bytes from baseaddress + offset"},
  {"RAM",       CMDfunction,-1,(void *)freeRam,NULL,      "Display the avalible ram memory in bytes"},
  // Digital IO commands
  {"PINMODE",   CMDfunction,-1,(void *)setPinMode,NULL,   "Set pin mode to INPUT,OUTPUT, or PULLUP. pin, mode"},
  {"DOUT",      CMDfunction,-1,(void *)setPin,NULL,       "Set output pin, HIGH, LOW, PULSE, SPULSE"},
  {"DIN",       CMDfunction,-1,(void *)getPin,NULL,       "Read input pin, HIGH, LOW"},
  // Analog IO command
  {"ADCRES",   CMDfunction,-1,(void *)setADCres,NULL,     "Set analog input resolution in bits"},
  {"DACRES",   CMDfunction,-1,(void *)setDACres,NULL,     "Set analog output resolution in bits"},
  {"ADC",      CMDfunction,-1,(void *)getADC,NULL,        "Read ADC input channel"},
  {"DAC",      CMDfunction,-1,(void *)setDAC,NULL,        "Write DAC output channel"},
  // Debug functions
  {"DEBUG",     CMDfunction,-1,(void *)callDebug,NULL,    "Function use for debug during development"},
  {"UPTIME",    CMDfunction,-1,(void *)upTime,NULL,       "Report system up time sinse last reboot"},
  {"RESET",     CMDfunction,-1,(void *)debug::softwareReset,NULL,"Causes the system to reboot"},
  {"UUID",      CMDfunction,-1,(void *)UUID,NULL,         "Report 128 bit UUID, in hex"},
  {"CPUTEMP",   CMDfunction,-1,(void *)cpuTemp,NULL,      "Report cpu temperature in C"},
  {NULL}
};
static CommandList debugList = {debugCmds, NULL};


debug::debug(commandProcessor *cmdP)
{
  cp = cmdP;
  db = this;
  debugFunction = NULL;
}

CommandList *debug::debugCommands(void)
{
  return &debugList;
}

void  debug::registerDebugFunction(void (*function)(void))
{
  debugFunction = function;
}

void debug::setAddress(uint32_t address)
{
  baseAddress = address;
}


