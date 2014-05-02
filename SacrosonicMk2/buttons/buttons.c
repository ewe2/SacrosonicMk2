#include "buttons.h"

void btn_init(Btn_struct * btnStruct){
    uint32_t rccPeriph = 0;
    switch(btnStruct->portLetter){
    case 'A':
        btnStruct->port = GPIOA;
        rccPeriph = RCC_AHB1Periph_GPIOA;
        break;
    case 'B':
        btnStruct->port = GPIOB;
        rccPeriph = RCC_AHB1Periph_GPIOB;
        break;
    case 'C':
        btnStruct->port = GPIOC;
        rccPeriph = RCC_AHB1Periph_GPIOC;
        break;
    case 'D':
        btnStruct->port = GPIOD;
        rccPeriph = RCC_AHB1Periph_GPIOD;
        break;
    case 'E':
        btnStruct->port = GPIOE;
        rccPeriph = RCC_AHB1Periph_GPIOE;
        break;
    case 'F':
        btnStruct->port = GPIOF;
        rccPeriph = RCC_AHB1Periph_GPIOF;
        break;
    case 'G':
        btnStruct->port = GPIOG;
        rccPeriph = RCC_AHB1Periph_GPIOG;
        break;
    case 'H':
        btnStruct->port = GPIOH;
        rccPeriph = RCC_AHB1Periph_GPIOH;
        break;
    case 'I':
        btnStruct->port = GPIOI;
        rccPeriph = RCC_AHB1Periph_GPIOI;
        break;
    default:
        rccPeriph = RCC_AHB1Periph_GPIOA;
        btnStruct->port = GPIOA;
    }


    RCC_AHB1PeriphClockCmd(rccPeriph, ENABLE);

    GPIO_InitTypeDef initStruct;
    initStruct.GPIO_Pin = (1 << btnStruct->pin);
    initStruct.GPIO_Mode = GPIO_Mode_IN;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    initStruct.GPIO_OType = GPIO_OType_OD;
    initStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(btnStruct->port, &initStruct);

    btnStruct->debounceCountdown = 0;
    btnStruct->oneShotTriggered = 0;
}

int btn_readCont(Btn_struct * btnStruct){
    uint8_t output = 0;
    if(btnStruct->port->IDR & (1 << btnStruct->pin)){ // button pushed
        btnStruct->debounceCountdown = btnStruct->debounceTime;
        output = 1;
    } else { // button not pushed
        if(btnStruct->debounceCountdown > 0){
            btnStruct->debounceCountdown -= (timer_getTimerTicks() - btnStruct->lastReadTime);
            output = 1;
        }
    }

    btnStruct->lastReadTime = timer_getTimerTicks();
    return output;
}

int btn_readOneShot(Btn_struct * btnStruct){
    if(btn_readCont(btnStruct) && !btnStruct->oneShotTriggered){
        btnStruct->oneShotTriggered = 1;
        return 1;
    } else if(!btn_readCont(btnStruct) && btnStruct->oneShotTriggered){
        btnStruct->oneShotTriggered = 0;
        return 0;
    } else {
        return 0;
    }
}
