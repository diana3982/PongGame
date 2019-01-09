#include <msp430.h>
#include "buzzer.h"
#include "libTimer.h"

#define MIN_PERIOD 1000
#define MAX_PERIOD 4000

static unsigned int period = 4000;
static signed int rate = 12000;

/* the following code is pulled from lab 2 */
void buzzer_init(){
  timerAUpmode();
  P2SEL2 &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6;
}

void buzzer_advance_frequency() 
{
  period += rate;
  if ((rate > 0 && (period > MAX_PERIOD)) || 
      (rate < 0 && (period < MIN_PERIOD))) {
    rate = -rate;
    period += (rate << 1);
  }
  buzzer_set_period(period);
}

void buzzer_set_period(short cycles)
{
  CCR0 = cycles; 
  CCR1 = cycles >> 1;		/* one half cycle */
}


void winner(int win){
switch(win){
 case 0:
  buzzer_advance_frequency(4000);
  break;
 case 1:
  buzzer_advance_frequency(6000);
  break;
 }
case 2:
break;
}
