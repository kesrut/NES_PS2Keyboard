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
#define KBD_CLK 29 /* PE3 */
#define KBD_DATA 28 /* PE2 */
#define BEGIN 0
#define CLK_START 1
#define START 1
#define STOP 3
#define PARITY 4
#define INIT 5

#define MAKE 0xE0
#define BREAK 0xF0

volatile unsigned int cnt=0;
volatile unsigned int clock=0;
volatile unsigned int data = 0xff ;
volatile unsigned int divider = 0x80 ;
volatile unsigned int cycles = 0 ; 
volatile unsigned int value = 0 ; 
volatile unsigned int got_data = 0 ;
volatile unsigned int state = BEGIN ;
volatile unsigned int kbd_state = INIT ; 

void setup()
{
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                SYSCTL_OSC_MAIN);
  Serial.begin(9600);
  pinMode(LATCH, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(KBD_CLK, INPUT_PULLUP);
  pinMode(DATA, OUTPUT) ;
  pinMode(KBD_DATA, INPUT) ;
  digitalWrite(DATA, LOW) ;
  attachInterrupt(LATCH, latch, RISING);
  attachInterrupt(CLK, clk, FALLING);
  attachInterrupt(KBD_CLK, kbd_clock, FALLING);
}

unsigned int _read()
{
   if (got_data)
   {
    got_data = 0 ; 
    return value ; 
  } 
  return 0 ; 
}

void loop()
{
  unsigned int keycode = _read() ;
  if (keycode)
  {
    Serial.println(keycode, HEX) ;
    switch (keycode)
    {
      case 0x1C:
      { // 'a' key - b nes
        data = (data & 0xBF) ; 
      } break ;
      case 0x1B:
      { // 's' key - a nes
        data = (data & 0x7F) ;
      } break ;
      case 0x5A:
      { // enter - start nes
        data = (data & 0xDF) ;
      } break; 
      case 0x29:
      {
        data = (data & 0xEF) ;
      } break ;
    }
    if (keycode == BREAK)
    {
      delay(4) ;
      unsigned int key = _read() ;
      switch (key)
      {
      case 0x1C:
      { // 'a' key - b nes
        data = (data |( ~0xBF)) ; 
      } break ;
      case 0x1B:
      { // 's' key - a nes
        data = (data | (~0x7F)) ;
      } break ;
      case 0x5A:
      { // enter - start nes
        data = (data | (~0xDF)) ;
      } break; 
       case 0x29:
      { // space - select
        data = (data | (~0xEF)) ;
      } break ;
    }
    }
    if (keycode == MAKE)
    {
      delay(4) ;
      unsigned int key = _read() ;
      if (key != BREAK)
      {
        switch (key)
        {
          case 0x75:
          { // up arrow
            data = (data & 0xF7) ; 
          } break ;
          case 0x6B:
          { // left arrow
            data = (data & 0xFD) ;
          } break ;
          case 0x72:
          { // down arrow
            data = (data & 0xFB) ;
          } break ;
          case 0x74:
          { // right arrow
            data = (data & 0xFE) ; 
          } break ;
        }
      }
      else
      {
        delay(4) ;
        unsigned int key = _read() ;
        switch (key)
        {
          case 0x75:
          { // up arrow
            data = (data | (~0xF7)) ; 
          } break ;
          case 0x6B:
          { // left arrow
            data = (data | (~0xFD)) ;
          } break ;
          case 0x72:
          { // down arrow
            data = (data | (~0xFB)) ;
          } break ;
          case 0x74:
          { // right arrow
            data = (data | (~0xFE)) ; 
          } break ;
      }
    }
  }
  }
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

void kbd_clock()
{
   if (kbd_state == INIT)
   {
     if (digitalRead(KBD_DATA) == LOW)
     {
       kbd_state = START ; 
       cycles = 0 ;
       got_data = 0 ;
       value = 0 ; 
       return ; 
     }
   }
   if (kbd_state == START)
   {
     value |= (digitalRead(KBD_DATA) << cycles) ;
     cycles++ ; 
     if (cycles == 8) kbd_state = PARITY ;
     return ;  
   }
   if (kbd_state == PARITY)
   {
     kbd_state = STOP ; 
     return ; 
   }
   if (kbd_state == STOP)
   {
     if (digitalRead(KBD_DATA) == HIGH)
     {
       kbd_state = INIT ; 
       got_data = 1 ; 
       return ; 
     }
   }  
}
