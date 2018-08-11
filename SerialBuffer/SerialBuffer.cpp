#include "SerialBuffer.h"


SerialBuffer::SerialBuffer()
{
}

SerialBuffer::~SerialBuffer()
{
}

void SerialBuffer::begin()
{
   tail = head = sbsize = 0;
}

size_t SerialBuffer::write(uint8_t by)
{
   if(sbsize == SB_SIZE) return(0);  // Full!
   buf[head++] = by;
   if(head >= SB_SIZE) head = 0;
   noInterrupts();
   sbsize++;
   interrupts();
   return(1);
}

size_t SerialBuffer::write(const uint8_t *ch, size_t sz)
{
   if(sbsize == SB_SIZE) return(0);  // Full!
   // Insert characters at head pointer
   int num = 0;
   for(int i=0; i < sz; i++)
   {
      buf[head++] = ch[i];
      num++;
      if(head >= SB_SIZE) head = 0;
      noInterrupts();
      sbsize++;
      if(sbsize == SB_SIZE) break;
      interrupts();
   }
   interrupts();
   return(num);
}

int SerialBuffer::available(void)
{
   return(sbsize);
}

int SerialBuffer::read(void)
{
   if(sbsize == 0) return(-1);	// empty
   int i = buf[tail++];
   if(tail >= SB_SIZE) tail = 0;
   noInterrupts();
   sbsize--;
   interrupts();
   return(i);
}

int SerialBuffer::peek(void)
{
}

void SerialBuffer::flush(void)
{
}

