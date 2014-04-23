#ifndef CS43L22_I2C_H_INCLUDED
#define CS43L22_I2C_H_INCLUDED

#include "stm32f4xx_conf.h"
#include "stm32f4xx_i2c.h"

#define CS43L22_I2C_PORT (I2C1)
#define CS43L22_I2C_ADDR (0x4a << 1) // already includes the left shift for writing to the bus

void cs43l22_i2c_initClocksAndPins();
void cs43l22_i2c_config();

void cs43l22_i2c_startSession();
void cs43l22_i2c_stopSession();

void cs43l22_i2c_startAndSendAddr(char readWrite);
void cs43l22_i2c_sendByte(uint8_t data);
void cs43l22_i2c_sendBytesAndStop(uint8_t * data, uint8_t length);
void cs43l22_i2c_writeByte(uint8_t mapByte, uint8_t data);
void cs43l22_i2c_writeBytes(uint8_t mapByte, uint8_t * data, uint8_t length);

void cs43l22_i2c_startReadMode(uint8_t mapByte);

uint8_t cs43l22_i2c_readByte(uint8_t mapByte);
void cs43l22_i2c_readBytes(uint8_t mapByte, uint8_t * data, uint8_t length);

void cs43l22_i2c_waitForFlag(uint32_t i2c_flag, uint8_t desiredValue);

void cs43l22_i2c_waitForEvent(uint32_t i2c_event);

enum cs43l22_i2c_ConnectionStatus {
    DEAD, // uninitialized
    READY, // initialized, no open session
    OPENED_READ, // initialized, open session with address sent and read mode set
    OPENED_WRITE // initialized, open session with address sent and write mode set
};

#endif
