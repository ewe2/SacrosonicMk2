#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "stm32f4xx_tim.h"

#define TIMER_CLOCK_SPEED 84000000

uint32_t timer_getTimerTicks();
float timer_convertTicksToSeconds(uint32_t ticks);

void timer_init();
void timer_delay(float seconds);

#endif /* TIME_H_INCLUDED */
