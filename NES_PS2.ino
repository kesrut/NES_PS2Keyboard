#include "Energia.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"

#include "driverlib/rom.h"
#include "driverlib/cpu.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#define LATCH 11
#define CLK 10
#define DATA 13

volatile unsigned int cnt=0;
volatile unsigned int clock=0;
volatile unsigned int data = 0xff ;
volatile unsigned int divider = 0x80 ;

#define BEGIN 0
#define CLK_START 1

volatile unsigned int state = BEGIN ;

void setup()
{
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                SYSCTL_OSC_MAIN);
  Serial.begin(9600);
  pinMode(LATCH, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, OUTPUT) ;
  digitalWrite(DATA, LOW) ;
  attachInterrupt(LATCH, latch, RISING);
  attachInterrupt(CLK, clk, FALLING);
}

void loop()
{
  
  if (Serial.available())
    {
      unsigned char keycode = Serial.read() ; 
      Serial.write(keycode) ;
          switch (keycode)
          {
             case 'w': // up
             {
               data = (data & 0xF7) ; 
             } break ; 
             case 's': // down
             {
               data = (data & 0xFB) ; 
             } break ; 
             case 'd': // forward
             {
               data = (data & 0xFE) ; 
             } break ; 
             case 'a': // backward
             {
               data = (data & 0xFD) ;
             } break ;
             case ';': // B
             {
               data = (data & 0xBF) ; 
             } break ;
             case '\'': // A
             {
               data = (data & 0x7F) ;
             } break ;
             case 0x13: // start
             {
               data = (data & 0xDF) ;
             } break ;
             case ']':
             {
               data = (data & 0xEF) ;
             } break ;
          }
        }
    delay(80) ;
    data = 0xFF;
}

void latch()
{
  if (state == BEGIN)
  {
    if ((data & divider) & 0xFF) digitalWrite(DATA, HIGH) ;
    else digitalWrite(DATA, LOW) ;
    divider = divider >> 1; 
    cnt = 0 ;
    state = CLK_START ;
  }
}

void clk()
{
  if (state == CLK_START)
  {
    if ((data & divider) & 0xff) digitalWrite(DATA, HIGH) ;
    else digitalWrite(DATA, LOW) ;
   cnt++ ;
   divider = divider >> 1 ;
   if (cnt == 8)
   {
     state = BEGIN ;
     divider = 0x80 ;
     digitalWrite(DATA, LOW) ;
   }
  } 
}
