#ifndef CS43L22_I2S_H_INCLUDED
#define CS43L22_I2S_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "stm32f4xx_spi.h"

#define CS43L22_I2S_PORT SPI3

#define CS43L22_I2S_PLL_N 197 // NOTE: ADJUSTED FOR CRUDDY EXTERNAL OSCILLATOR.
#define CS43L22_I2S_PLL_R 2

void cs43l22_i2s_initClocksAndPins();
void cs43l22_i2s_config();

void cs43l22_i2s_outputSampleWhenReady(int16_t sample);
int cs43l22_i2s_attemptOutputSample(int16_t sample);



#endif /* CS43L22_I2S_H_INCLUDED */
