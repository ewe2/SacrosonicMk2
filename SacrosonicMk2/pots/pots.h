#ifndef POTS_H_INCLUDED
#define POTS_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"

#define POTS_NUMBER 5
#define POTS_OVERSAMPLING 8
#define POTS_DMA_ARRAY_SIZE (POTS_NUMBER * POTS_OVERSAMPLING)

#define POTS_OUTPUT_MIN 0
#define POTS_OUTPUT_MAX 4095

#define POTS_TOTAL_OUTPUT_MAX (POTS_OVERSAMPLING * POTS_OUTPUT_MAX)

__IO uint16_t pots_values[POTS_DMA_ARRAY_SIZE]; // dma output array

void pots_dmaInit();
void pots_adcInit();
void pots_start();
void pots_initAndStart();

float pots_getMappedAverage(uint16_t pot);

#endif
