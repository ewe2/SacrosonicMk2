#include "cs43l22_i2c.h"

enum cs43l22_i2c_ConnectionStatus cs43l22_i2c_connection = DEAD;

void cs43l22_i2c_initClocksAndPins() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitTypeDef pinInitStruct;
    pinInitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9;
    pinInitStruct.GPIO_Mode = GPIO_Mode_AF;
    pinInitStruct.GPIO_OType = GPIO_OType_OD;
    pinInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    pinInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &pinInitStruct);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_I2C1);
}

void cs43l22_i2c_config(){
    I2C_InitTypeDef i2cInitStruct;
    i2cInitStruct.I2C_ClockSpeed = 100000;
    i2cInitStruct.I2C_Mode = I2C_Mode_I2C;
    i2cInitStruct.I2C_OwnAddress1 = 0x00;
    i2cInitStruct.I2C_Ack = I2C_Ack_Disable;
    i2cInitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2cInitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_Init(I2C1,&i2cInitStruct);

    I2C_Cmd(I2C1,ENABLE);

    cs43l22_i2c_connection = READY;
}

void cs43l22_i2c_startSession(){
    assert_param(cs43l22_i2c_connection == READY);

    cs43l22_i2c_waitForFlag(I2C_FLAG_BUSY, 0); // wait just in case another send is still in progress

    I2C_GenerateSTART(CS43L22_I2C_PORT, ENABLE); // start condition (master pulls SDA low)
    cs43l22_i2c_waitForEvent(I2C_EVENT_MASTER_MODE_SELECT); // wait for master-side verification
}

void cs43l22_i2c_stopSession(){
    assert_param(cs43l22_i2c_connection == OPENED_READ || cs43l22_i2c_connection == OPENED_WRITE);

    I2C_GenerateSTOP(CS43L22_I2C_PORT,ENABLE);
    cs43l22_i2c_connection = READY;
}

void cs43l22_i2c_startAndSendAddr(char readWrite){
    assert_param(cs43l22_i2c_connection == READY && (readWrite == 'r' || readWrite == 'w'));

    cs43l22_i2c_startSession();

    if(readWrite == 'r'){
        I2C_Send7bitAddress(CS43L22_I2C_PORT, CS43L22_I2C_ADDR, I2C_Direction_Receiver); // send chip address
        cs43l22_i2c_waitForEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED); // wait for slave acknowledge
        cs43l22_i2c_connection = OPENED_READ;
    } else { // free to assume 'w' because we've asserted parameters at the start of the function
        I2C_Send7bitAddress(CS43L22_I2C_PORT, CS43L22_I2C_ADDR, I2C_Direction_Transmitter); // send chip address
        cs43l22_i2c_waitForEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); // wait for slave acknowledge
        cs43l22_i2c_connection = OPENED_WRITE;
    }
}

void cs43l22_i2c_sendByte(uint8_t  data){
    assert_param(cs43l22_i2c_connection == OPENED_WRITE || cs43l22_i2c_connection == OPENEED_READ);

    I2C_SendData(CS43L22_I2C_PORT, data); // send byte
    cs43l22_i2c_waitForEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED); // wait for master-side verification
}

void cs43l22_i2c_sendBytesAndStop(uint8_t * data, uint8_t length){
    assert_param(cs43l22_i2c_connection == OPENED_WRITE);

    int i = 0;
    for(;i < length; i++) cs43l22_i2c_sendByte(data[i]);

    cs43l22_i2c_stopSession();
}

void cs43l22_i2c_writeByte(uint8_t mapByte, uint8_t data){
    cs43l22_i2c_startAndSendAddr('w');
    cs43l22_i2c_sendByte(mapByte);
    cs43l22_i2c_sendByte(data);

    I2C_GenerateSTOP(CS43L22_I2C_PORT,ENABLE);
    cs43l22_i2c_connection = READY;
}

// Sends a write of bytes to the I2C1 device
void cs43l22_i2c_writeBytes(uint8_t mapByte, uint8_t * data, uint8_t length) {
    cs43l22_i2c_startAndSendAddr('w');

    if(length > 1) mapByte |= (1 << 7);

    cs43l22_i2c_sendByte(mapByte);
    cs43l22_i2c_sendBytesAndStop(data, length);
}

void cs43l22_i2c_startReadMode(uint8_t mapByte){
    cs43l22_i2c_startAndSendAddr('w');
    cs43l22_i2c_sendByte(mapByte);

    cs43l22_i2c_stopSession();

    cs43l22_i2c_startAndSendAddr('r');
}

uint8_t cs43l22_i2c_readByte(uint8_t mapByte) {
    cs43l22_i2c_startReadMode(mapByte);

    I2C_AcknowledgeConfig(CS43L22_I2C_PORT,DISABLE);

    cs43l22_i2c_waitForEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);

    cs43l22_i2c_stopSession();

    return I2C_ReceiveData(CS43L22_I2C_PORT);
}

void cs43l22_i2c_readBytes(uint8_t mapByte, uint8_t * data, uint8_t length){
    int i = 0;
    for(; i < length; i++){
        data[i] = cs43l22_i2c_readByte(mapByte+i);
    }

    /* For some reason I can't get this to work properly, so we are using multiple calls of cs43l22_readByte() instead;
    assert_param(cs43l22_i2c_connection == OPENED);
    mapByte |= (1 << 7);


    I2C_AcknowledgeConfig(CS43L22_I2C_PORT,ENABLE);

    cs43l22_startReadMode(mapByte);

    int i = 0;
    for(; i < length; i++){
        if(i == length - 2) I2C_AcknowledgeConfig(CS43L22_I2C_PORT,DISABLE);

        waitForEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);
        data[i] = I2C_ReceiveData(CS43L22_I2C_PORT);
    }

    cs43l22_stopSession(); */
}

void cs43l22_i2c_waitForFlag(uint32_t i2c_flag, uint8_t desiredValue){
    while(I2C_GetFlagStatus(CS43L22_I2C_PORT,i2c_flag) != desiredValue);
}

void cs43l22_i2c_waitForEvent(uint32_t i2c_event){
    while(!I2C_CheckEvent(CS43L22_I2C_PORT, i2c_event));
}
