/*
  PPM.h - PPM header file, definition of methods and attributes.
  
  This class supports the generation of a PPM pulse sequence. 
  The DueTimer class is used and the version updated by Gordon
  Anderson is required.
  This class can be used to generate any custom pulse sequence.
  
  Created by Gordon Anderson, November, 2013.
*/
#ifdef __arm__

#ifndef PPM_h
#define PPM_h

#include "DueTimer.h"

class PPM
{
	friend void RAmatch_Handler();
	friend void RBmatch_Handler();
	friend void RCmatch_Handler();
	friend uint32_t ApplyRangeLimits(uint32_t value);
protected:
	// Represents the timer id (index for the array of Timer structs in DueTimer)
	static int timer;
	// Working variables for PPM signal generation
	static uint32_t NumChannels;
	static uint32_t *Channels;
	static uint32_t SyncWidth;
	static uint32_t MinWidth;
	static uint32_t MaxWidth;
	static bool	ActiveHigh;
	static int	AuxOutSignal;
	
	static int TIOApins[9];
	static int TIOBpins[9];
public:
	// Needs to be public, because the handlers are outside class:
	static void (*callbackPeriod)();
	static void (*callbackPulse)();

	PPM(int _timer);
	PPM attachInterruptPeriod(void (*isr)());
	PPM attachInterruptPulse(void (*isr)());
	PPM detachInterrupts();
	PPM start(double frequency = 40.0, uint8_t ClockDivisor = TC_CMR_TCCLKS_TIMER_CLOCK2);
	PPM stop();
        PPM setSyncWidth(uint32_t Count);
        PPM setSyncSense(bool _ActiveHigh);
        PPM setNumChannels(uint32_t _NumChannels);
        PPM setChannelArray(uint32_t *ChannelArray);
        PPM setMinWidth(uint32_t Count);
        PPM setMaxWidth(uint32_t Count);
        PPM setAuxOut(int signal);

	double getClockFrequency();
	int getCurrentChannel();
};

#endif

#else
	#error Oops! Trying to include PPM on non Due device?
#endif
