#include "cs43l22_i2s.h"

void cs43l22_i2s_initClocksAndPins(){
    assert_param(CS43L22_I2S_PORT == SPI3); // if this isn't the case none of this will make sense.

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA
                           | RCC_AHB1Periph_GPIOC
                           , ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    RCC_PLLI2SCmd(ENABLE);

    GPIO_InitTypeDef pinInitStruct;

    // I2S WS, via the SPI3 peripheral
    pinInitStruct.GPIO_Pin = GPIO_Pin_4;
    pinInitStruct.GPIO_Mode = GPIO_Mode_AF;
    pinInitStruct.GPIO_OType = GPIO_OType_PP;
    pinInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    pinInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&pinInitStruct);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_SPI3);

    // I2S MCLK, SCLK and SDIN, also via SPI3
    pinInitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_12;
    pinInitStruct.GPIO_Mode = GPIO_Mode_AF;
    pinInitStruct.GPIO_OType = GPIO_OType_PP;
    pinInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    pinInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &pinInitStruct);
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3);
}

void cs43l22_i2s_config(){
    // configuring I2S
    I2S_InitTypeDef i2sInitStruct;
    i2sInitStruct.I2S_AudioFreq = I2S_AudioFreq_48k;
    i2sInitStruct.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
    i2sInitStruct.I2S_Mode = I2S_Mode_MasterTx;
    i2sInitStruct.I2S_DataFormat = I2S_DataFormat_16b;
    i2sInitStruct.I2S_Standard = I2S_Standard_Phillips;
    i2sInitStruct.I2S_CPOL = I2S_CPOL_Low;
    I2S_Init(CS43L22_I2S_PORT,&i2sInitStruct);
    I2S_Cmd(CS43L22_I2S_PORT, ENABLE);
}

void cs43l22_i2s_outputSample(int16_t sample){
    while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)); // wait for the bus to be ready
    SPI_I2S_SendData(CS43L22_I2S_PORT,sample); // send the sample
}

int cs43l22_i2s_attemptOutputSample(int16_t sample){
    if(SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)){
        SPI_I2S_SendData(CS43L22_I2S_PORT,sample); // send the sample
        return 1;
    } else {
        return 0;
    }
}
