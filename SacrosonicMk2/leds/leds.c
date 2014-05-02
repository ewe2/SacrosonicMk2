#include "leds.h"

void led_set(int led, int state){
    if(led == LED_ORANGE || led == LED_GREEN || led == LED_RED || led == LED_BLUE){
        if(state == 1) GPIOD->BSRRL = (1 << led);
        if(state == 0) GPIOD->BSRRH = (1 << led);
    }
}

void led_setAll(int orange, int green, int red, int blue){
    led_set(LED_ORANGE,orange);
    led_set(LED_GREEN,green);
    led_set(LED_RED,red);
    led_set(LED_BLUE,blue);
}

void led_init(){
    GPIO_InitTypeDef initStruct;


    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    initStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12; // we want to configure all LED GPIO pins
	initStruct.GPIO_Mode = GPIO_Mode_OUT; 		// we want the pins to be an output
	initStruct.GPIO_Speed = GPIO_Speed_50MHz; 	// this sets the GPIO modules clock speed
	initStruct.GPIO_OType = GPIO_OType_PP; 	// this sets the pin type to push / pull (as opposed to open drain)
	initStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 	// this sets the pullup / pulldown resistors to be inactive
	GPIO_Init(GPIOD, &initStruct);

	led_setAll(0,1,0,1);
}

