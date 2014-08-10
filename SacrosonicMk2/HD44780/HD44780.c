#include "HD44780.h"

void HD44780_setDataPins(uint8_t data) {
    GPIO_WriteBit(HD44780_DATA_PORT,HD44780_DATA_0,(data >> 0) & 1);
    GPIO_WriteBit(HD44780_DATA_PORT,HD44780_DATA_1,(data >> 1) & 1);
    GPIO_WriteBit(HD44780_DATA_PORT,HD44780_DATA_2,(data >> 2) & 1);
    GPIO_WriteBit(HD44780_DATA_PORT,HD44780_DATA_3,(data >> 3) & 1);
}

void HD44780_pulseEnable() {
    GPIO_SetBits(HD44780_CTRL_PORT, HD44780_CTRL_EN);
    timer_delay(HD44780_EN_PULSE);
    GPIO_ResetBits(HD44780_CTRL_PORT, HD44780_CTRL_EN);
    timer_delay(HD44780_EN_PULSE);
}

void HD44780_waitForBusy() {

    GPIO_InitTypeDef initStruct;
    GPIO_StructInit(&initStruct);
    initStruct.GPIO_Pin = HD44780_DATA_3;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HD44780_DATA_PORT, &initStruct);

    GPIO_ResetBits(HD44780_CTRL_PORT, HD44780_CTRL_RS);
    GPIO_SetBits(HD44780_CTRL_PORT, HD44780_CTRL_RW);

    HD44780_setDataPins(0);

    uint8_t busy = 1;
    while(busy){
        GPIO_SetBits(HD44780_CTRL_PORT, HD44780_CTRL_EN);
        timer_delay(HD44780_EN_PULSE);
        busy = GPIO_ReadInputDataBit(HD44780_DATA_PORT,HD44780_DATA_3);
        GPIO_ResetBits(HD44780_CTRL_PORT, HD44780_CTRL_EN);
        timer_delay(HD44780_EN_PULSE);
        HD44780_pulseEnable(); // we don't need to read the low nibble, just let it pass.
    }

    initStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(HD44780_DATA_PORT, &initStruct);
}

void HD44780_writeNibble(uint8_t data, uint8_t isCommand) {
    HD44780_setDataPins(data);
    HD44780_pulseEnable();
}

void HD44780_write(uint8_t data, uint8_t isCommand) {
    HD44780_waitForBusy();

    GPIO_ResetBits(HD44780_CTRL_PORT,HD44780_CTRL_RW);
    GPIO_WriteBit(HD44780_CTRL_PORT,HD44780_CTRL_RS,!isCommand);

    HD44780_writeNibble((data >> 4) & 0xf, isCommand);
    HD44780_writeNibble((data >> 0) & 0xf, isCommand);
}

void HD44780_writeCommand(uint8_t command) {
    HD44780_write(command,1);
}

void HD44780_writeData(uint8_t data) {
    HD44780_write(data,0);
}

void HD44780_switchToLine(uint8_t line) {
    switch(line) {
    case 1:
        HD44780_writeCommand(HD44780_CMD_MOV_CRSR | HD44780_LINE_1);
        break;
    case 2:
        HD44780_writeCommand(HD44780_CMD_MOV_CRSR | HD44780_LINE_2);
        break;
    }
}

void HD44780_fillScreen(char c) {
    uint8_t row, column;
    for(row = 0; row < 2; row++) {
        HD44780_switchToLine(row+1);
        for(column = 0; column < 16; column++) {
            HD44780_writeData(c);
        }
    }
}

void HD44780_writeString(char* aString) {
    int i = 0;
    while(aString[i] != '\0') {
        HD44780_writeData(aString[i++]);
    }
}

void HD44780_initGpio() {
    GPIO_InitTypeDef initStruct;

    RCC_AHB1PeriphClockCmd(HD44780_DATA_PORT_AHB, ENABLE);
    initStruct.GPIO_Pin = HD44780_DATA_0
                          | HD44780_DATA_1
                          | HD44780_DATA_2
                          | HD44780_DATA_3;
    initStruct.GPIO_Mode = GPIO_Mode_OUT;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    initStruct.GPIO_OType = GPIO_OType_PP;
    initStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(HD44780_DATA_PORT, &initStruct);

    GPIO_ResetBits(HD44780_DATA_PORT,GPIO_Pin_All);


    RCC_AHB1PeriphClockCmd(HD44780_CTRL_PORT_AHB, ENABLE);
    initStruct.GPIO_Pin = HD44780_CTRL_EN
                          | HD44780_CTRL_RS
                          | HD44780_CTRL_RW;
    initStruct.GPIO_Mode = GPIO_Mode_OUT;
    initStruct.GPIO_Speed = GPIO_Speed_50MHz;
    initStruct.GPIO_OType = GPIO_OType_PP;
    initStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(HD44780_CTRL_PORT, &initStruct);

    GPIO_ResetBits(HD44780_CTRL_PORT,GPIO_Pin_All);
}

void HD44780_initDisplay() {
    // Wait 20ms for LCD to power up
    timer_delay(0.02);

    // Write D7-0 = 30 hex, with RS = 0
    GPIO_ResetBits(HD44780_CTRL_PORT, HD44780_CTRL_RS);
    GPIO_ResetBits(HD44780_CTRL_PORT, HD44780_CTRL_RW);
    HD44780_setDataPins(0x30 >> 4);
    timer_delay(0.001);
    HD44780_pulseEnable();
    // Wait 5ms
    timer_delay(0.005);

    // Write D7-0 = 30 hex, with RS = 0, again
    HD44780_pulseEnable(); // data and rs already in place
    // Wait 200us
    timer_delay(0.0002);

    // Write D7-0 = 30 hex, with RS = 0, one more time
    HD44780_pulseEnable(); // data and rs/rw already in place
    // Wait 200us
    timer_delay(0.0002);

    // this configuration should perhaps be made more flexible
    HD44780_writeNibble(0x20 >> 4, 1); //set interface to 4-bit
    HD44780_writeCommand(0x08); // enable display, hide cursor
    HD44780_writeCommand(0x01); // clear and home display
    HD44780_writeCommand(0x06); // move cursor right on character input
    HD44780_writeCommand(0x0C); // enable display
}

void HD44780_init() {
    HD44780_initGpio();
    HD44780_initDisplay();
    HD44780_writeString("Hello!");
}
