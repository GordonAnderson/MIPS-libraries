#include <Arduino.h>
#include <inttypes.h>

#define SB_SIZE 512

class SerialBuffer: public Stream
{
   public:
      SerialBuffer();
      ~SerialBuffer();
      void begin();
	  virtual size_t write(uint8_t);
	  virtual size_t write(const uint8_t *, size_t);
	  virtual int available(void);
	  virtual int read(void);
	  virtual int peek(void);
	  virtual void flush(void);
	  
   private:
      uint8_t  buf[SB_SIZE];
      uint16_t head;
      uint16_t tail;
      uint16_t sbsize;
};