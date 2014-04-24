#include "timer.h"

// gets the current timer ticks count
uint32_t timer_getTimerTicks(){
    return TIM2->CNT;
}

// converts the given ticks to seconds
float timer_convertTicksToSeconds(uint32_t ticks){
    float output = (float)ticks / (float)TIMER_CLOCK_SPEED;
    return output;
}

// initializes and starts TIM2 running at 84mhz with a period of 0xffffffff (roughly 51 seconds)
void timer_init(){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef initStruct;
    initStruct.TIM_Prescaler = 1 - 1; // keep it running at 84 mhz
    initStruct.TIM_CounterMode = TIM_CounterMode_Up;
    initStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    initStruct.TIM_Period = UINT32_MAX;
    TIM_TimeBaseInit(TIM2,&initStruct);
    TIM_Cmd(TIM2,ENABLE);
}

// delays for the given seconds. behavior when given negative input or input above 51 undefined
void timer_delay(float seconds){
    uint32_t ticks = seconds * TIMER_CLOCK_SPEED;

    uint32_t startTime = TIM2->CNT;
    uint32_t endTime = TIM2->CNT + ticks;

    if(startTime > endTime){ // our endTime is beyond an overflow, wait for the overflow first
        while(TIM2->CNT > endTime);
    }

    while(TIM2->CNT < endTime);
}
