#include "cs43l22.h"

// initializes the GPIO pin (PD4) that is connected to the CS43L22 reset
void cs43l22_initResetPin(){
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef pinInitStruct;
    pinInitStruct.GPIO_Pin = GPIO_Pin_4;
    pinInitStruct.GPIO_Mode = GPIO_Mode_OUT;
    pinInitStruct.GPIO_OType = GPIO_OType_PP;
    pinInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    pinInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &pinInitStruct);
}

// Central init function. First calls a bunch of other init functions and then configures the CS43L22 for 16-bit@48kHz stereo output
void cs43l22_init(){
    cs43l22_initResetPin();

    cs43l22_powerOff();

    cs43l22_i2c_initClocksAndPins();
    cs43l22_i2s_initClocksAndPins();

    cs43l22_i2c_config();
    cs43l22_i2s_config();

    cs43l22_powerOn();

    // delay for a bit to let the cs43l22 get fired up
    int i = 0;
    for(i = 0; i < 1000000; i++) asm("NOP");

    cs43l22_i2c_writeByte(CS43L22_REG_PLAYBACK_CTRL1,0x01); // mute the output

    // Required initialization settings (as described mysteriously on page 32 of cs43l22 ref)
    cs43l22_i2c_writeByte(0x00,0x99);
    cs43l22_i2c_writeByte(0x47,0x80);
    cs43l22_i2c_writeByte(0x32,cs43l22_i2c_readByte(0x32) | (1<<7)); // set bit 7
    cs43l22_i2c_writeByte(0x32,cs43l22_i2c_readByte(0x32) & ~(1<<7)); // clear bit 7
    cs43l22_i2c_writeByte(0x00,0x00);

    // turn headphone channels ALWAYS ON and speaker channels ALWAYS OFF
    cs43l22_i2c_writeByte(CS43L22_REG_POWER_CTL2, 0xaf);

    // set headphone gain to 0.6047 (50-ish%) and playback volume control of all channels to ganged
    cs43l22_i2c_writeByte(CS43L22_REG_PLAYBACK_CTRL1, 0x00);

    // set clock mode to auto detect
    cs43l22_i2c_writeByte(CS43L22_REG_CLOCKING_CTL, 0x80);

    // sets the interface to slave mode i2s, and sample size to 16 bit
    cs43l22_i2c_writeByte(CS43L22_REG_INTERFACE_CTL1, 0x07);

    // Turn off analog soft ramp and zero cross (I have no idea what they even are)
    cs43l22_i2c_writeByte(CS43L22_REG_ANALOG_SET, cs43l22_i2c_readByte(CS43L22_REG_ANALOG_SET) & ~(0xf));

    // Turn off limiters
    cs43l22_i2c_writeByte(CS43L22_REG_LIMIT_CTRL1,0x00);

    // set volume of both channels (to 0dB)
    cs43l22_i2c_writeByte(CS43L22_REG_PCMA_VOL, 0x00);
    cs43l22_i2c_writeByte(CS43L22_REG_PCMB_VOL, 0x00);

    // turn up the bass! (set high gain on bass, I'm not sure if this is really necessary/reasonable)
    cs43l22_i2c_writeByte(CS43L22_REG_TONE_CTRL, 0x0f);

    // turn on output power
    cs43l22_i2c_writeByte(CS43L22_REG_POWER_CTL1, 0x9e);
}

// powers on the unit by setting the pin that is attached to the CS43L22 reset
void cs43l22_powerOn(){
    GPIO_SetBits(GPIOD,GPIO_Pin_4);
}

// powers off the unit by clearing the pin that is attached to the CS43L22 reset
void cs43l22_powerOff(){
    GPIO_ResetBits(GPIOD,GPIO_Pin_4);
}

// Tries to output a sample and will not stop trying until it succeeds. Use with care.
void cs43l22_outputSample(int16_t sample){
    cs43l22_i2s_outputSample(sample);
}

// Tries to output a sample once and then returns 1 if successful, 0 if not.
int cs43l22_attemptOutputSample(int16_t sample){
    return cs43l22_i2s_attemptOutputSample(sample);
}
