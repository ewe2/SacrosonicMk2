#include "pots.h"

void pots_dmaInit(){
    int i = 0;
    for(i = 0; i < POTS_DMA_ARRAY_SIZE; i++){ // initialize array with 0s to make sure we don't get broken ADC-reads
        pots_values[i] = 0x0000;
    }

    DMA_DeInit(DMA2_Stream0);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_InitTypeDef dmaInitStruct;
    dmaInitStruct.DMA_Channel = DMA_Channel_0;
    dmaInitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&ADC1->DR);
    dmaInitStruct.DMA_Memory0BaseAddr = (uint32_t)pots_values;
    dmaInitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dmaInitStruct.DMA_BufferSize = POTS_DMA_ARRAY_SIZE;
    dmaInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmaInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dmaInitStruct.DMA_Mode = DMA_Mode_Circular;
    dmaInitStruct.DMA_Priority = DMA_Priority_High;
    dmaInitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dmaInitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dmaInitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dmaInitStruct.DMA_PeripheralBurst = DMA_MemoryBurst_Single;
    DMA_Init(DMA2_Stream0,&dmaInitStruct);
    DMA_Cmd(DMA2_Stream0,ENABLE);
}

void pots_adcInit(){
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpioInitStruct;
    gpioInitStruct.GPIO_Pin = GPIO_Pin_0
                            | GPIO_Pin_1
                            | GPIO_Pin_2
                            | GPIO_Pin_3;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_AN;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInitStruct);
    GPIO_Init(GPIOC, &gpioInitStruct);


    ADC_CommonInitTypeDef adcCommonInitStruct;
    adcCommonInitStruct.ADC_Mode = ADC_Mode_Independent;
    adcCommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
    adcCommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    adcCommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&adcCommonInitStruct);

    ADC_InitTypeDef adcInitStruct;
    adcInitStruct.ADC_Resolution = ADC_Resolution_12b;
    adcInitStruct.ADC_ScanConvMode = ENABLE;
    adcInitStruct.ADC_ContinuousConvMode = ENABLE;
    adcInitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    adcInitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    adcInitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    adcInitStruct.ADC_NbrOfConversion = POTS_NUMBER;
    ADC_Init(ADC1,&adcInitStruct);

    ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_480Cycles); // pc0
    ADC_RegularChannelConfig(ADC1,ADC_Channel_11,2,ADC_SampleTime_480Cycles); // pc1
    ADC_RegularChannelConfig(ADC1,ADC_Channel_12,3,ADC_SampleTime_480Cycles); // pc2
    ADC_RegularChannelConfig(ADC1,ADC_Channel_13,4,ADC_SampleTime_480Cycles); // pc3

    ADC_RegularChannelConfig(ADC1,ADC_Channel_0,5,ADC_SampleTime_480Cycles); // pa0
    ADC_RegularChannelConfig(ADC1,ADC_Channel_1,6,ADC_SampleTime_480Cycles); // pa1
    ADC_RegularChannelConfig(ADC1,ADC_Channel_2,7,ADC_SampleTime_480Cycles); // pa2
    ADC_RegularChannelConfig(ADC1,ADC_Channel_3,8,ADC_SampleTime_480Cycles); // pa3

    ADC_ContinuousModeCmd(ADC1,ENABLE);
}

void pots_start(){
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1,ENABLE);
    ADC_Cmd(ADC1,ENABLE);

    ADC_SoftwareStartConv(ADC1);
}

void pots_initAndStart(){
    pots_adcInit();
    pots_dmaInit();
    pots_start();
}

float pots_getMappedAverage(uint16_t pot){
    assert_param(pot < POTS_NUMBER);

    uint16_t total = 0;

    int sample = 0;
    for(; sample < POTS_OVERSAMPLING; sample++){
        total += pots_values[pot + sample * POTS_NUMBER];
    }

    return (float)total / POTS_TOTAL_OUTPUT_MAX;
}


int pots_readIfActive(uint16_t pot, float * output){
    if(!pots_readSinceSwitch[pot]){
        pots_lastActiveRead[pot] = pots_getMappedAverage(pot);
        pots_readSinceSwitch[pot] = 1;
        pots_isActive[pot] = 0;
    } else {
        float newValue = pots_getMappedAverage(pot);
        if(pots_isActive[pot] // if the pot is active
           || newValue + POTS_ACTIVATION_THRESHOLD < pots_lastActiveRead[pot] // or the pot is past the activation threshold
           || newValue - POTS_ACTIVATION_THRESHOLD > pots_lastActiveRead[pot]){
            pots_isActive[pot] = 1;
            *output = newValue;
        }
    }
    return pots_isActive[pot];
}

void pots_switchFunction(){
    int i = 0;
    for(i = 0; i < POTS_NUMBER; i++){
        pots_readSinceSwitch[i] = 0;
    }
}
