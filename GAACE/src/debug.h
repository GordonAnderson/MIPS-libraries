#ifndef DEBUG_H_
#define DEBUG_H_

#include <arduino.h>
#include "commandProcessor.h"

class debug
{
//  friend void callDebug(void);
	public:
		debug(commandProcessor *cmdP);
    void  registerDebugFunction(void (*function)(void));
    CommandList *debugCommands(void);
    void setAddress(uint32_t address);
    
    static void softwareReset(void);
	private:
};

#endif

