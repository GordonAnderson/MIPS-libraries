#include "DIhandler.h"
#include <stdarg.h>
#include <stdio.h>
//#include <vector.h>

static void DI_Q_ISR(void);
static void DI_R_ISR(void);
static void DI_S_ISR(void);
static void DI_T_ISR(void);
static void DI_U_ISR(void);
static void DI_V_ISR(void);
static void DI_W_ISR(void);
static void DI_X_ISR(void);

int DIhandler::DIpin[8] = {30,12,50,51,53,28,46,17};
void (*DIhandler::DI_ISR[8])(void) = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
void (*DI_ISRs[8])(void) = {DI_Q_ISR,DI_R_ISR,DI_S_ISR,DI_T_ISR,DI_U_ISR,DI_V_ISR,DI_W_ISR,DI_X_ISR};
int DIhandler::NumHandlers = 0;
DIhandler *DIhandler::handlers[MaxDIhandlers];

DIhandler::DIhandler(void)
{
   // The constructor of the class DIhandler 
   di = 0;
   mode = -1;
   userISR = NULL;
   if(NumHandlers < 10) handlers[NumHandlers++] = this;
}

DIhandler::~DIhandler(void)
{
   int Index,i;
   
   detach();
   NumHandlers=0;
   for(i=0;i<MaxDIhandlers;i++)
   {
      if(handlers[i] == this) handlers[i] = NULL;
      if(handlers[i] != NULL) NumHandlers++;
   }
}

bool DIhandler::attached(char DI, int Mode,void (*isr)(void))
{
   int Index,i;
   DIhandler *d=NULL;
   
   // Make sure we are in the list else exit with error
   for(i=0;i<MaxDIhandlers;i++) if(handlers[i] == this) d = this;
   if(d != this) return false;
   Index = DI - 'Q';
   if((Index < 0) || (Index > 7)) return false;
   // If we are already attached then exit.
   if(userISR != NULL) return false;
   di = DI;
   mode = Mode;
   if(DI_ISR[Index] == NULL)
   {
      userISR = isr;
      attachInterrupt(DIpin[Index], DI_ISRs[Index], CHANGE);
      DI_ISR[Index] = DI_ISRs[Index];
   }
   else userISR = isr;
   return true;
}

void DIhandler::detach(void)
{
   int Index,i;
   
   if(di == 0) return;
   Index = di - 'Q';
   if((Index < 0) || (Index > 7)) return;
   di = 0;
   userISR = NULL;
   mode = -1;
   // If this input is no longer used then detach the port interrupt
   for(i=0;i<MaxDIhandlers;i++)
   {
      if(handlers[i] != NULL) if(handlers[i]->di == ('Q'+Index)) return;
   }
   if(DIpin[Index] != NULL) detachInterrupt(DIpin[Index]);
   DI_ISR[Index] = NULL;
}

bool DIhandler::activeLevel(void)
{
   int Index,i;
   
   Index = di - 'Q';
   if((di==0) || (mode==-1)) return true;
   if(mode == CHANGE) return true;
   if((digitalRead(DIpin[Index]) == HIGH) && ((mode == RISING) || (mode == HIGH))) return true;
   if((digitalRead(DIpin[Index]) == LOW) && ((mode == FALLING) || (mode == LOW))) return true;
   return false;
}

void DI_Generic_ISR(int Index)
{
   int i;
   
   for(i=0;i<MaxDIhandlers;i++)
   {
      if(DIhandler::handlers != NULL)
      {
         if(DIhandler::handlers[i]->di != ('Q'+Index)) continue; 
         if(DIhandler::handlers[i]->userISR  == NULL) continue;
         if(DIhandler::handlers[i]->mode == CHANGE) DIhandler::handlers[i]->userISR();
         else if(DIhandler::handlers[i]->mode == HIGH) DIhandler::handlers[i]->userISR();
         else if(DIhandler::handlers[i]->mode == LOW) DIhandler::handlers[i]->userISR();
         else if((DIhandler::handlers[i]->mode == RISING) && (digitalRead(DIhandler::DIpin[Index]) == HIGH)) DIhandler::handlers[i]->userISR();
         else if((DIhandler::handlers[i]->mode == FALLING) && (digitalRead(DIhandler::DIpin[Index]) == LOW)) DIhandler::handlers[i]->userISR();
      }
   }
}

void DI_Q_ISR(void)
{
   DI_Generic_ISR(0);
}
void DI_R_ISR(void)
{
   DI_Generic_ISR(1);
}
void DI_S_ISR(void)
{
   DI_Generic_ISR(2);
}
void DI_T_ISR(void)
{
   DI_Generic_ISR(3);
}
void DI_U_ISR(void)
{
   DI_Generic_ISR(4);
}
void DI_V_ISR(void)
{
   DI_Generic_ISR(5);
}
void DI_W_ISR(void)
{
   DI_Generic_ISR(6);
}
void DI_X_ISR(void)
{
   DI_Generic_ISR(7);
}

