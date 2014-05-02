#ifndef POTS_H_INCLUDED
#define POTS_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"

#define POTS_NUMBER 5
#define POTS_OVERSAMPLING 16
#define POTS_DMA_ARRAY_SIZE (POTS_NUMBER * POTS_OVERSAMPLING)

#define POTS_OUTPUT_MIN 0
#define POTS_OUTPUT_MAX 4095

#define POTS_TOTAL_OUTPUT_MAX (POTS_OVERSAMPLING * POTS_OUTPUT_MAX)

#define POTS_ACTIVATION_THRESHOLD 0.02

__IO uint16_t pots_values[POTS_DMA_ARRAY_SIZE]; // dma output array

float pots_lastActiveRead[POTS_NUMBER];
uint8_t pots_readSinceSwitch[POTS_NUMBER];
uint8_t pots_isActive[POTS_NUMBER];

void pots_dmaInit();
void pots_adcInit();
void pots_start();
void pots_initAndStart();

float pots_getMappedAverage(uint16_t pot);
int pots_readIfActive(uint16_t pot, float * output);

void pots_switchFunction();

#endif
