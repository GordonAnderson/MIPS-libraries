#ifndef CHARALLOCATE_H_
#define CHARALLOCATE_H_

#include <arduino.h>

// This is a character array allocation class. When this class
// is inialized it creates a buffer used to allocate char arrays.
class charAllocate
{
	public:
		charAllocate(int caSize = 512);
		~charAllocate();
		char *allocate(int size);
		void free(char *mem);
		void clear(void);
		void defrag(void);
		int  available(void);
	private:
		int  size;
		char *buffer;
};

#endif
