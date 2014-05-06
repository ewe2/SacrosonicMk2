#include "midi.h"

void midi_initBuffers() {
    int i = 0;
    for(i = 0; i < MIDI_BUFFER_RAW_SIZE; i++) {
        midi_rawBuffer[i] = midi_rawBufferShadow[i] = 0xff;
    }

    midi_rawBufferIndex = 0;
    midi_msgBufferIndex = 0;
}

void midi_initGpio() {
    RCC_AHB1PeriphClockCmd(MIDI_GPIO_PORT_RCC, ENABLE);

    GPIO_InitTypeDef gpioInitStruct;
    gpioInitStruct.GPIO_Pin = MIDI_GPIO_PIN_RX;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MIDI_GPIO_PORT, &gpioInitStruct);

    GPIO_PinAFConfig(MIDI_GPIO_PORT,MIDI_GPIO_PIN_RX_SRC,GPIO_AF_USART2);
}

void midi_initUSART() {
    RCC_APB1PeriphClockCmd(MIDI_USART_PORT_RCC, ENABLE);

    USART_InitTypeDef usartInitStruct;

    usartInitStruct.USART_BaudRate = MIDI_USART_BAUDRATE;
    usartInitStruct.USART_WordLength = USART_WordLength_8b;
    usartInitStruct.USART_StopBits = USART_StopBits_1;
    usartInitStruct.USART_Parity = USART_Parity_No;
    usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usartInitStruct.USART_Mode = USART_Mode_Rx;
    USART_Init(MIDI_USART_PORT,&usartInitStruct);
    USART_Cmd(MIDI_USART_PORT,ENABLE);
}

void midi_initDMA() {
    DMA_DeInit(MIDI_DMA_STREAM);
    RCC_AHB1PeriphClockCmd(MIDI_DMA_RCC, ENABLE);

    DMA_InitTypeDef dmaInitStruct;
    dmaInitStruct.DMA_Channel = MIDI_DMA_CHANNEL;
    dmaInitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&MIDI_USART_PORT->DR);
    dmaInitStruct.DMA_Memory0BaseAddr = (uint32_t)midi_rawBuffer;
    dmaInitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dmaInitStruct.DMA_BufferSize = MIDI_BUFFER_RAW_SIZE;
    dmaInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dmaInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dmaInitStruct.DMA_Mode = DMA_Mode_Circular;
    dmaInitStruct.DMA_Priority = DMA_Priority_High;
    dmaInitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dmaInitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dmaInitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dmaInitStruct.DMA_PeripheralBurst = DMA_MemoryBurst_Single;
    DMA_Init(MIDI_DMA_STREAM,&dmaInitStruct);
    DMA_Cmd(MIDI_DMA_STREAM,ENABLE);

    USART_DMACmd(MIDI_USART_PORT, MIDI_USART_DMA_REQ, ENABLE);
}

void midi_init() {
    midi_initBuffers();
    midi_initGpio();
    midi_initUSART();
    midi_initDMA();

}

uint8_t midi_getNumberOfDataBytesForMsgType(uint8_t msgType) {
    switch(msgType) {
    case MIDI_MSG_TYPE_NOTE_OFF:
        return 2;
    case MIDI_MSG_TYPE_NOTE_ON:
        return 2;
    case MIDI_MSG_TYPE_AFTERTOUCH:
        return 2;
    case MIDI_MSG_TYPE_CTRL_CHANGE:
        return 2;
    case MIDI_MSG_TYPE_PROGRAM_CHANGE:
        return 1;
    case MIDI_MSG_TYPE_CHAN_PRESSURE:
        return 1;
    case MIDI_MSG_TYPE_PITCH_WHEEL:
        return 2;
    default:
        return 0;
    }
}

void midi_catchUpWithRawBuffer() {
    uint8_t currentByte = 0;
    Midi_basicMsg * currentMsg;
    while(midi_rawBufferShadow[midi_rawBufferIndex] != midi_rawBuffer[midi_rawBufferIndex]) {

        if(midi_msgBufferIndex < MIDI_BUFFER_MSGS_SIZE) { // space left in buffer?
            currentByte = midi_rawBuffer[midi_rawBufferIndex];
            currentMsg = &midi_msgBuffer[midi_msgBufferIndex];

            if(currentByte > 127) { // status byte?
                currentMsg->msgType = currentByte & 0xf0;
                currentMsg->lowNibble = currentByte & 0x0f;
                currentMsg->numberOfDataBytes = midi_getNumberOfDataBytesForMsgType(currentMsg->msgType);
                currentMsg->dataByteIndex = 0;

            } else if(currentMsg->msgType != 0 // current message has been initialized?
                    && currentMsg->dataByteIndex < currentMsg->numberOfDataBytes) { // still space left for more databytes?
                currentMsg->dataBytes[currentMsg->dataByteIndex++] = currentByte;
                if(currentMsg->dataByteIndex == currentMsg->numberOfDataBytes) midi_msgBufferIndex++;// msg is complete, increment msgBuffer index
            }

            midi_rawBufferShadow[midi_rawBufferIndex++] = currentByte;
            if(midi_rawBufferIndex == MIDI_BUFFER_RAW_SIZE) midi_rawBufferIndex = 0;
        } else { // no space left in buffer
            break; // break from loop and come back next function call
        }
    }
}

int midi_getMsgIfAble(Midi_basicMsg * msg) {
    midi_catchUpWithRawBuffer();
    if(midi_msgBufferIndex > 0){ // a message is ready
        *msg = midi_msgBuffer[--midi_msgBufferIndex];

        midi_msgBuffer[midi_msgBufferIndex].msgType = 0; // clear msgType to indicate that it has been read.
        return 1;
    }
    return 0;
}
