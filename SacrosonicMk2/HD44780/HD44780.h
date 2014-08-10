#ifndef HD44780_H_INCLUDED
#define HD44780_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"
#include <stdint.h>
#include "../timer/timer.h"

#define HD44780_DATA_PORT GPIOD
#define HD44780_DATA_PORT_AHB RCC_AHB1Periph_GPIOD
#define HD44780_DATA_0 GPIO_Pin_0
#define HD44780_DATA_1 GPIO_Pin_1
#define HD44780_DATA_2 GPIO_Pin_2
#define HD44780_DATA_3 GPIO_Pin_3

#define HD44780_CTRL_PORT GPIOE
#define HD44780_CTRL_PORT_AHB RCC_AHB1Periph_GPIOE
#define HD44780_CTRL_RS GPIO_Pin_0
#define HD44780_CTRL_RW GPIO_Pin_1
#define HD44780_CTRL_EN GPIO_Pin_2

#define HD44780_CMD_MOV_CRSR 0x80

#define HD44780_LINE_1 0x00
#define HD44780_LINE_2 0x40

#define HD44780_EN_PULSE 0.0000005 // this is in seconds

void HD44780_setDataPins(uint8_t data);
void HD44780_pulseEnable();

void HD44780_waitForBusy();

void HD44780_writeNibble(uint8_t data, uint8_t isCommand);
void HD44780_write(uint8_t data, uint8_t isCommand);

void HD44780_writeCommand(uint8_t command);
void HD44780_writeData(uint8_t data);

void HD44780_switchToLine(uint8_t line);
void HD44780_fillScreen(char c);
void HD44780_writeString(char * aString);

void HD44780_initGpio();
void HD44780_initDisplay();
void HD44780_init();


#endif /* HD44780_H_INCLUDED */
