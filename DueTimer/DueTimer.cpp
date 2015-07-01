/*
  DueTimer.cpp - Implementation of Timers defined on DueTimer.h
  For instructions, go to https://github.com/ivanseidel/DueTimer

  Created by Ivan Seidel Gomes, March, 2013.
  Modified by Philipp Klaus, June 2013.
  Thanks to stimmer (from Arduino forum), for coding the "timer soul" (Register stuff)
  Released into the public domain.
*/

#include "DueTimer.h"

const DueTimer::Timer DueTimer::Timers[9] = {
	{TC0,0,TC0_IRQn},
	{TC0,1,TC1_IRQn},
	{TC0,2,TC2_IRQn},
	{TC1,0,TC3_IRQn},
	{TC1,1,TC4_IRQn},
	{TC1,2,TC5_IRQn},
	{TC2,0,TC6_IRQn},
	{TC2,1,TC7_IRQn},
	{TC2,2,TC8_IRQn},
};

void (*DueTimer::callbacks[9])() = {};
void (*DueTimer::callbacksRA[9])() = {};
void (*DueTimer::callbacksRB[9])() = {};
double DueTimer::_frequency[9] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
double DueTimer::ClockFrequency[9]={0,0,0,0,0,0,0,0,0};

/*
	Initializing all timers, so you can use them like this: Timer0.start();
*/
DueTimer Timer(0);

DueTimer Timer0(0);
DueTimer Timer1(1);
DueTimer Timer2(2);
DueTimer Timer3(3);
DueTimer Timer4(4);
DueTimer Timer5(5);
DueTimer Timer6(6);
DueTimer Timer7(7);
DueTimer Timer8(8);

DueTimer::DueTimer(int _timer){
	/*
		The constructor of the class DueTimer 
	*/

	timer = _timer;
}

DueTimer DueTimer::getAvailable(){
	/*
		Return the first timer with no callback set
	*/

	for(int i = 0; i < 9; i++){
		if(!callbacks[i])
			return DueTimer(i);
	}
	// Default, return Timer0;
	return DueTimer(0);
}

DueTimer DueTimer::attachInterrupt(void (*isr)()){
	/*
		Links the function passed as argument to the timer of the object
	*/

	callbacks[timer] = isr;

	return *this;
}

DueTimer DueTimer::attachInterruptRA(void (*isr)()){
	/*
		Links the function passed as argument to the timer of the object
	*/

	callbacksRA[timer] = isr;

	return *this;
}

DueTimer DueTimer::attachInterruptRB(void (*isr)()){
	/*
		Links the function passed as argument to the timer of the object
	*/

	callbacksRB[timer] = isr;

	return *this;
}

DueTimer DueTimer::detachInterrupt(){
	/*
		Links the function passed as argument to the timer of the object
	*/

	stop(); // Stop the currently running timer

	callbacks[timer] = NULL;
	callbacksRA[timer] = NULL;
	callbacksRB[timer] = NULL;

	return *this;
}

DueTimer DueTimer::start(long microseconds, uint8_t ClockDivisor){
	/*
		Start the timer
		If a period is set, then sets the period and start the timer
	*/

	if(microseconds > 0)
		setPeriod(microseconds,ClockDivisor);
	
	if(_frequency[timer] <= 0)
		setFrequency(1,ClockDivisor);

	NVIC_ClearPendingIRQ(Timers[timer].irq);
	NVIC_EnableIRQ(Timers[timer].irq);
	
	TC_Start(Timers[timer].tc, Timers[timer].channel);

	return *this;
}

DueTimer DueTimer::stop(){
	/*
		Stop the timer
	*/

	NVIC_DisableIRQ(Timers[timer].irq);
	
	TC_Stop(Timers[timer].tc, Timers[timer].channel);

	return *this;
}

DueTimer DueTimer::setTIOAeffect(uint32_t count, uint32_t effect)
{
	// Get current timer configuration
	Timer t = Timers[timer];

	t.tc->TC_CHANNEL[t.channel].TC_RA = count;
	
//	t.tc->TC_BCR = 0; this works!
	
	t.tc->TC_CHANNEL[t.channel].TC_CMR = (t.tc->TC_CHANNEL[t.channel].TC_CMR 
					     & ~(TC_CMR_ACPA_Msk | TC_CMR_ACPC_Msk | TC_CMR_AEEVT_Msk | TC_CMR_ASWTRG_Msk)) 
					     | effect;
					     
	// Enable the RA Compare Interrupt if effect != 0...
	if(effect !=0)
	{
	   t.tc->TC_CHANNEL[t.channel].TC_IER |= TC_IER_CPAS;
	   t.tc->TC_CHANNEL[t.channel].TC_IDR &= ~TC_IER_CPAS;
        }
        else
        {
	   t.tc->TC_CHANNEL[t.channel].TC_IER &= ~TC_IER_CPAS;
	   t.tc->TC_CHANNEL[t.channel].TC_IDR |= TC_IER_CPAS;
        }

	return *this;
}

DueTimer DueTimer::incRA(uint32_t count)
{
	// Get current timer configuration
	Timer t = Timers[timer];

	t.tc->TC_CHANNEL[t.channel].TC_RA += count;
	return *this;
}

DueTimer DueTimer::setTIOBeffect(uint32_t count, uint32_t effect)
{
	// Get current timer configuration
	Timer t = Timers[timer];

	t.tc->TC_CHANNEL[t.channel].TC_RB = count;
	
	t.tc->TC_CHANNEL[t.channel].TC_CMR = (t.tc->TC_CHANNEL[t.channel].TC_CMR 
					     & ~(TC_CMR_BCPB_Msk | TC_CMR_BCPC_Msk | TC_CMR_BEEVT_Msk | TC_CMR_BSWTRG_Msk)) 
					     | effect;

	// Make sure EEVT in CMR is not 0, TIOB will not happen if it is 0
	if((t.tc->TC_CHANNEL[t.channel].TC_CMR & TC_CMR_EEVT_Msk) == 0) 
		t.tc->TC_CHANNEL[t.channel].TC_CMR |= TC_CMR_EEVT_XC0;
	
	// Enable the RB Compare Interrupt if effect != 0...
	if(effect !=0)
	{
	   t.tc->TC_CHANNEL[t.channel].TC_IER |= TC_IER_CPBS;
	   t.tc->TC_CHANNEL[t.channel].TC_IDR &= ~TC_IER_CPBS;
        }
        else
        {
	   t.tc->TC_CHANNEL[t.channel].TC_IER &= ~TC_IER_CPBS;
	   t.tc->TC_CHANNEL[t.channel].TC_IDR |= TC_IER_CPBS;
        }
	return *this;
}

DueTimer DueTimer::incRB(uint32_t count)
{
	// Get current timer configuration
	Timer t = Timers[timer];

	t.tc->TC_CHANNEL[t.channel].TC_RB += count;
	return *this;
}

uint8_t DueTimer::bestClock(double frequency, uint32_t& retRC, uint8_t ClockDivisor){
	/*
		Pick the best Clock, thanks to Ogle Basil Hall!

		Timer		Definition
		TIMER_CLOCK1	MCK /  2
		TIMER_CLOCK2	MCK /  8
		TIMER_CLOCK3	MCK / 32
		TIMER_CLOCK4	MCK /128
	*/
	struct {
		uint8_t flag;
		uint8_t divisor;
	} clockConfig[] = {
		{ TC_CMR_TCCLKS_TIMER_CLOCK1,   2 },
		{ TC_CMR_TCCLKS_TIMER_CLOCK2,   8 },
		{ TC_CMR_TCCLKS_TIMER_CLOCK3,  32 },
		{ TC_CMR_TCCLKS_TIMER_CLOCK4, 128 }
	};
	float ticks;
	float error;
	int clkId = 3;
	int bestClock = 3;
	float bestError = 1.0;
	do
	{
	        if(ClockDivisor == clockConfig[clkId].flag)
	        {
	           bestClock = clkId;
	           break;
	        }
		ticks = (float) VARIANT_MCK / frequency / (float) clockConfig[clkId].divisor;
		error = abs(ticks - round(ticks));
		if (abs(error) < bestError)
		{
			bestClock = clkId;
			bestError = error;
		}
	} while (clkId-- > 0);
	ticks = (float) VARIANT_MCK / frequency / (float) clockConfig[bestClock].divisor;
	retRC = (uint32_t) round(ticks);
	return clockConfig[bestClock].flag;
}


DueTimer DueTimer::setFrequency(double frequency, uint8_t ClockDivisor){
	/*
		Set the timer frequency (in Hz)
	*/

	// Prevent negative frequencies
	if(frequency <= 0) { frequency = 1; }

	// Remember the frequency
	_frequency[timer] = frequency;

	// Get current timer configuration
	Timer t = Timers[timer];

	uint32_t rc = 0;
	uint8_t clock;

	// Tell the Power Management Controller to disable 
	// the write protection of the (Timer/Counter) registers:
	pmc_set_writeprotect(false);

	// Enable clock for the timer
	pmc_enable_periph_clk((uint32_t)t.irq);

	// Find the best clock for the wanted frequency
	clock = bestClock(frequency, rc, ClockDivisor);
	
	// Set the clock frequency variable
	if(clock == TC_CMR_TCCLKS_TIMER_CLOCK1) ClockFrequency[timer] = (double)VARIANT_MCK / 2.0;
	if(clock == TC_CMR_TCCLKS_TIMER_CLOCK2) ClockFrequency[timer] = (double)VARIANT_MCK / 8.0;
	if(clock == TC_CMR_TCCLKS_TIMER_CLOCK3) ClockFrequency[timer] = (double)VARIANT_MCK / 32.0;
	if(clock == TC_CMR_TCCLKS_TIMER_CLOCK4) ClockFrequency[timer] = (double)VARIANT_MCK / 128.0;


	// Set up the Timer in waveform mode which creates a PWM
	// in UP mode with automatic trigger on RC Compare
	// and sets it up with the determined internal clock as clock input.
	TC_Configure(t.tc, t.channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | clock);
	// Reset counter and fire interrupt when RC value is matched:
	TC_SetRC(t.tc, t.channel, rc);
	// Enable the RC Compare Interrupt...
	t.tc->TC_CHANNEL[t.channel].TC_IER=TC_IER_CPCS;
	// ... and disable all others.
	t.tc->TC_CHANNEL[t.channel].TC_IDR=~TC_IER_CPCS;

	return *this;
}

DueTimer DueTimer::setPeriod(long microseconds, uint8_t ClockDivisor){
	/*
		Set the period of the timer (in microseconds)
	*/
	// Convert period in microseconds to frequency in Hz
	double frequency = 1000000.0 / microseconds;	
	setFrequency(frequency,ClockDivisor);
	return *this;
}

double DueTimer::getFrequency(){
	/*
		Get current time frequency
	*/
	return _frequency[timer];
}

double DueTimer::getClockFrequency(){
	return ClockFrequency[timer];
}

long DueTimer::getPeriod(){
	/*
		Get current time period
	*/
	return 1.0/getFrequency()*1000000;
}


/*
	Implementation of the timer callbacks defined in 
	arduino-1.5.2/hardware/arduino/sam/system/CMSIS/Device/ATMEL/sam3xa/include/sam3x8e.h
*/
void TC0_Handler(){
	int i = TC_GetStatus(TC0, 0);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[0]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[0]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[0]();
}
void TC1_Handler(){
	int i = TC_GetStatus(TC0, 1);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[1]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[1]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[1]();
}
void TC2_Handler(){
	int i = TC_GetStatus(TC0, 2);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[2]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[2]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[2]();
}
void TC3_Handler(){
	int i = TC_GetStatus(TC1, 0);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[3]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[3]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[3]();
}
void TC4_Handler(){
	int i = TC_GetStatus(TC1, 1);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[4]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[4]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[4]();
}
void TC5_Handler(){
	int i = TC_GetStatus(TC1, 2);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[5]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[5]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[5]();
}
void TC6_Handler(){
	int i = TC_GetStatus(TC2, 0);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[6]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[6]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[6]();
}
void TC7_Handler(){
	int i = TC_GetStatus(TC2, 1);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[7]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[7]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[7]();
}
void TC8_Handler(){
	int i = TC_GetStatus(TC2, 2);
	if(i & TC_SR_CPAS) DueTimer::callbacksRA[8]();
	if(i & TC_SR_CPBS) DueTimer::callbacksRB[8]();
	if(i & TC_SR_CPCS) DueTimer::callbacks[8]();
}
