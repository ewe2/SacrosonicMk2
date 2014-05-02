#ifndef BUTTONS_H_INCLUDED
#define BUTTONS_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "../timer/timer.h"

typedef struct {
    char portLetter;
    uint8_t pin;
    int32_t debounceTime;

    GPIO_TypeDef * port;
    uint32_t lastReadTime;
    int32_t debounceCountdown;
    uint8_t oneShotTriggered;
} Btn_struct;

void btn_init(Btn_struct * btnStruct);
int btn_readCont(Btn_struct * btnStruct);
int btn_readOneShot(Btn_struct * btnStruct);

#endif /* BUTTONS_H_INCLUDED */
