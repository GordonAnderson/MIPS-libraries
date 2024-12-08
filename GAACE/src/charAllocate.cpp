#include "charAllocate.h"

charAllocate::charAllocate(int caSize)
{
	size = caSize;
	buffer = (char *)malloc(sizeof(char) * caSize);
  uint16_t  header = caSize - 2;
  buffer[0] = header & 0xFF;
	buffer[1] = (header>>8) & 0xFF;
}

charAllocate::~charAllocate()
{
  free(buffer);
  size = 0;
}

void charAllocate::clear(void)
{
  uint16_t  header = size - 2;
  buffer[0] = header & 0xFF;
	buffer[1] = (header>>8) & 0xFF;
}

char *charAllocate::allocate(int num)
{
	uint16_t  header;
	int       ptr=0;
  char      *buf;

  if(buffer == NULL) return NULL;
	header = buffer[ptr] | buffer[ptr+1] << 8;
	while(((header & 0x8000) != 0) || ((header & 0x7FFF) < num))
	{
  	ptr += (header&0x7FFF)+2;
   	if(ptr >= size) return NULL;
   	header = buffer[ptr] | buffer[ptr+1] << 8; 
	}
	// Here with a free block
	// Allocate block and mark remaining as a new free block
	int blkSize = (header&0x7FFF);
	if((blkSize - num) < 3) num = blkSize;
	header = num | 0x8000;
	buffer[ptr] = header & 0xFF;
	buffer[ptr+1] = (header>>8) & 0xFF;
	buf = &buffer[ptr+2];
	if(blkSize > num)
	{
		ptr += (header&0x7FFF)+2;
		header = blkSize - num - 2;
		buffer[ptr] = header & 0xFF;
		buffer[ptr+1] = (header>>8) & 0xFF;
	}
	return(buf);
}

void charAllocate::free(char *buf)
{
  if(buffer == NULL) return;
	// Get index into buffer
	uint32_t ptr = (uint32_t)buf - (uint32_t)buffer - 1;
  if((ptr < 0) || (ptr > (uint32_t)size)) return;
  if((buffer[ptr] & 0x80) == 0) return;
	buffer[ptr] &= 0x7F;
}

// Returns the largest avalible free block size in bytes
int charAllocate::available(void)
{
	uint16_t  header;
	int       ptr=0;
  int       i = 0;

  if(buffer == NULL) return 0;
 	header = buffer[ptr] | buffer[ptr+1] << 8;
	while(true)
  {
    if((header & 0x8000) == 0) if(header > i) i = header;
  	ptr += (header&0x7FFF)+2;
   	if(ptr >= size) return i;
   	header = buffer[ptr] | buffer[ptr+1] << 8; 
  }
  return 0;
}

void charAllocate::defrag(void)
{
	uint16_t  header,nextHeader;
	int       ptr=0, next;
  
  if(buffer == NULL) return;
 	header = buffer[ptr] | buffer[ptr+1] << 8;
	while(true)
  {
    if((header & 0x8000) == 0)
    {
      // See if the next block is free and combine them
      next = ptr;
      next += (header&0x7FFF)+2;
      if(next >= size) return;
      nextHeader = buffer[next] | buffer[next+1] << 8;
      if((nextHeader & 0x8000) == 0) 
      {
        header        += nextHeader + 2;
		    buffer[ptr]   =  header & 0xFF;
		    buffer[ptr+1] =  (header>>8) & 0xFF;
      }
    }
    else
    {
  	  ptr += (header&0x7FFF)+2;
   	  if(ptr >= size) return;
   	  header = buffer[ptr] | buffer[ptr+1] << 8; 
    }
  }
}
