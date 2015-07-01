/*
  PPM.cpp - Implementation of PPM defined on PPM.h.
  
  This class supports the generation of a PPM pulse sequence. 
  The DueTimer class is used and the version updated by Gordon
  Anderson is required.
  This class can be used to generate any custom pulse sequence.
  There can only be one of these objects created a any one time,
  most variables are static becuse this object include interrupt
  service routines.
  
  Created by Gordon Anderson, November, 2013.
*/

#include "PPM.h"	
   void (*PPM::callbackPeriod)() = NULL;
   void (*PPM::callbackPulse)() = NULL;

   int PPM::timer=-1;
   uint32_t  PPM::NumChannels;
   uint32_t *PPM::Channels;
   uint32_t  PPM::SyncWidth = 3500;
   uint32_t  PPM::MinWidth = 1000;
   uint32_t  PPM::MaxWidth = 30000;
   bool	     PPM::ActiveHigh = true;
   int	     PPM::AuxOutSignal=-1;
// Pin numbers for output pins for each timer, -1 indicates not avaliable for use
   int PPM::TIOApins[9] = {2,-1,-1,-1,-1,-1,5,3,11};
   int PPM::TIOBpins[9] = {13,-1,-1,-1,-1,-1,4,10,12};
// Interrupt handlers
   void RAmatch_Handler();
   void RBmatch_Handler();
   void RCmatch_Handler();
// Misc variables
   bool SyncActive;
   int  ChannelIndex;
   bool SyncActiveAux;
   int  ChannelIndexAux;

// The constructor of the class PPM
PPM::PPM(int _timer)
{
   if(timer != -1) return; // There can only be one of these objects!
   timer = _timer;
   NumChannels = 0;
   *Channels = NULL;
   ActiveHigh = true;
}
PPM PPM::attachInterruptPeriod(void (*isr)())
{
   callbackPeriod = isr;
   return *this;
}
PPM PPM::attachInterruptPulse(void (*isr)())
{
   callbackPulse = isr;
   return *this;
}
PPM PPM::detachInterrupts()
{
   stop(); // Stop the currently running PPM
   callbackPeriod = NULL;
   callbackPulse = NULL;
   return *this;
}
PPM PPM::start(double frequency, uint8_t ClockDivisor)
{
   DueTimer Timer(timer);
   
   // Define the timer and enable the output pins
   if(TIOApins[timer] != -1) PIO_Configure(g_APinDescription[TIOApins[timer]].pPort,
        				   g_APinDescription[TIOApins[timer]].ulPinType,
        				   g_APinDescription[TIOApins[timer]].ulPin,
        				   g_APinDescription[TIOApins[timer]].ulPinConfiguration);
   if(TIOBpins[timer] != -1) PIO_Configure(g_APinDescription[TIOBpins[timer]].pPort,
        				   g_APinDescription[TIOBpins[timer]].ulPinType,
        				   g_APinDescription[TIOBpins[timer]].ulPin,
        				   g_APinDescription[TIOBpins[timer]].ulPinConfiguration);
   // Setup the interrupt handlers
   Timer.attachInterruptRA(RAmatch_Handler);
   Timer.attachInterruptRB(RBmatch_Handler);
   Timer.attachInterrupt(RCmatch_Handler);
   // Start the timer
   Timer.setFrequency(frequency,ClockDivisor);
   Timer.start(0);
   
   return *this;
}
PPM PPM::stop()
{
   DueTimer Timer(timer);

   timer = -1;
   Timer.stop();
   return *this;
}
PPM PPM::setSyncWidth(uint32_t Count)
{
   SyncWidth = Count;
   return *this;
}
PPM PPM::setSyncSense(bool _ActiveHigh)
{
   ActiveHigh = _ActiveHigh;
   return *this;
}
PPM PPM::setNumChannels(uint32_t _NumChannels)
{
   NumChannels = _NumChannels;
   return *this;
}
PPM PPM::setChannelArray(uint32_t *ChannelArray)
{
   Channels = ChannelArray;
   return *this;
}
PPM PPM::setMinWidth(uint32_t Count)
{
   MinWidth = Count;
   return *this;
}
PPM PPM::setMaxWidth(uint32_t Count)
{
   MaxWidth = Count;
   return *this;
}
// This function defines the aux out signal. 
// -1 disables the output
//  0 set the output to PPM but alway active high
//  n = channel number with index starting at 1
PPM PPM::setAuxOut(int signal)
{
   AuxOutSignal = signal;
   return *this;
}

double PPM::getClockFrequency()
{
   DueTimer Timer(timer);

   return Timer.getClockFrequency();
}

int PPM::getCurrentChannel()
{
   return(ChannelIndex);
}

uint32_t ApplyRangeLimits(uint32_t value)
{
   if(value > PPM::MaxWidth) return(PPM::MaxWidth);
   if(value < PPM::MinWidth) return(PPM::MinWidth);
   return(value);
}
//
// Interrupt handlers
//

// Gereration of PPM output signal on TIOA
void RAmatch_Handler()
{
   DueTimer Timer(PPM::timer);


   if(SyncActive)
   {
      if(ChannelIndex >= PPM::NumChannels) 
      {
         // This happens one time per period and happens at the end of the
         // PPM position data, so call the period callback and exit.
         if(PPM::callbackPeriod != NULL) PPM::callbackPeriod();
         return;
      }
      Timer.incRA(ApplyRangeLimits(PPM::Channels[ChannelIndex++]) - PPM::SyncWidth);
      SyncActive = false;
      // Here after the sync pluse goes inactive, so call the pulse callback
      if(PPM::callbackPulse != NULL) PPM::callbackPulse();
      return;
   }
   Timer.incRA(PPM::SyncWidth);
   SyncActive = true;
}

void RBmatch_Handler()
{
   DueTimer Timer(PPM::timer);

   if(PPM::AuxOutSignal == 0)
   {
      if(SyncActiveAux)
      {
         if(ChannelIndexAux >= PPM::NumChannels) return;
         Timer.incRB(ApplyRangeLimits(PPM::Channels[ChannelIndexAux++]) - PPM::SyncWidth);
         SyncActiveAux = false;
         return;
      }
      Timer.incRB(PPM::SyncWidth);
      SyncActiveAux = true;
   }
}

void RCmatch_Handler()
{
   DueTimer Timer(PPM::timer);

// Enable the generation of a PPM frame on the TIOA output
   if(PPM::ActiveHigh)
      Timer.setTIOAeffect(PPM::SyncWidth,TC_CMR_ACPA_TOGGLE | TC_CMR_ACPC_SET);
   else
      Timer.setTIOAeffect(PPM::SyncWidth,TC_CMR_ACPA_TOGGLE | TC_CMR_ACPC_CLEAR);
   SyncActive = true;
   ChannelIndex = 0;
// Enable the selected signal output on TIOB
   if(PPM::AuxOutSignal == -1)
   {
      Timer.setTIOBeffect(0,0);		// Shut it off!
   }
   else if(PPM::AuxOutSignal == 0)
   {
      Timer.setTIOBeffect(PPM::SyncWidth,TC_CMR_BCPB_TOGGLE | TC_CMR_BCPC_SET);
      SyncActiveAux = true;
      ChannelIndexAux = 0;
   }
   else if(PPM::AuxOutSignal <= PPM::NumChannels)
   {
      Timer.setTIOBeffect(ApplyRangeLimits(PPM::Channels[PPM::AuxOutSignal-1]),TC_CMR_BCPB_TOGGLE | TC_CMR_BCPC_SET);
   }
}
