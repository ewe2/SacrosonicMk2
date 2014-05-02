#ifndef LEDS_H_INCLUDED
#define LEDS_H_INCLUDED

#include "stm32f4xx_conf.h"

#define LED_ORANGE 13
#define LED_GREEN 12
#define LED_RED 14
#define LED_BLUE 15

void led_set(int led, int state);
void led_setAll(int orange, int green, int red, int blue);
void led_init();

#endif /* LEDS_H_INCLUDED */
