#ifndef HX8347_H_
#define HX8347_H_

#define USE_MBED 1

//#if ARDUINO < 165
//#define USE_GFX_KBV
//#include "ADA_GFX_kbv.h"
//#else
#include "Adafruit_GFX.h"
//#endif

class HX8347 : public Adafruit_GFX {

 public:
//#if USE_MBED
//#if defined(TARGET_LPC1768)
  HX8347(uint8_t CS, uint8_t RS, uint8_t RST);
//#else
//  HX8347(uint8_t CS=D10, uint8_t RS=D7, uint8_t RST=D9);
//#endif
//#else
//  HX8347();
//#endif
  void     reset(void);                                       // you only need the constructor
  void     begin(uint16_t ID = 0x7575);                       // you only need the constructor
  void     disableDisplay(bool state);
  void     pushColor(uint16_t color);
  virtual void     drawPixel(int16_t x, int16_t y, uint16_t color);  // and these three
  void     WriteCmdData(uint16_t cmd, uint16_t dat);                 // public methods !!!
  uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
  uint16_t readID(void)     { return _lcd_ID; }

  virtual  void     fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  virtual  void     drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { fillRect(x, y, 1, h, color); }
  virtual  void     drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { fillRect(x, y, w, 1, color); }
  virtual  void     fillScreen(uint16_t color)                                     { fillRect(0, 0, _width, _height, color); }
  virtual  void     setRotation(uint8_t r);
  virtual  void     invertDisplay(boolean i);

  uint16_t readReg(uint16_t reg);
  int16_t  readGRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h);
  uint16_t readPixel(int16_t x, int16_t y) { uint16_t color; readGRAM(x, y, &color, 1, 1); return color; }
  void     setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1);
  void     pushColors(uint16_t *block, int16_t n, bool first);
  void     pushColors(uint8_t *block, int16_t n, bool first);
  void     pushColors(const uint8_t *block, int16_t n, bool first, bool bigend = false);
  void     vertScroll(int16_t top, int16_t scrollines, int16_t offset);

  uint8_t     spiwrite(uint8_t);
	
 protected:
 
 private:
#if USE_MBED
	  #define _spi     SPI
    uint8_t      _lcd_pin_cs, _lcd_pin_rs, _lcd_pin_reset;
#endif
    uint16_t        _lcd_ID;
};

#endif