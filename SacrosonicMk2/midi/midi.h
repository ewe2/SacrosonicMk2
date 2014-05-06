#ifndef MIDI_H_INCLUDED
#define MIDI_H_INCLUDED

#define MIDI_USART_PORT         USART2
#define MIDI_USART_PORT_RCC     RCC_APB1Periph_USART2
#define MIDI_USART_BAUDRATE     31250
#define MIDI_USART_DMA_REQ      USART_DMAReq_Rx

#define MIDI_GPIO_PORT          GPIOD
#define MIDI_GPIO_PORT_RCC      RCC_AHB1Periph_GPIOD
#define MIDI_GPIO_PIN_RX        GPIO_Pin_6
#define MIDI_GPIO_PIN_RX_SRC    GPIO_PinSource6
#define MIDI_GPIO_PIN_RX_AF     GPIO_AF_USART2

#define MIDI_DMA_STREAM         DMA1_Stream5
#define MIDI_DMA_RCC            RCC_AHB1Periph_DMA1
#define MIDI_DMA_CHANNEL        DMA_Channel_4

#define MIDI_BASIC_MSG_DATABYTES_MAX 2
#define MIDI_MSG_TYPE_NOTE_OFF          0x80
#define MIDI_MSG_TYPE_NOTE_ON           0x90
#define MIDI_MSG_TYPE_AFTERTOUCH        0xA0
#define MIDI_MSG_TYPE_CTRL_CHANGE       0xB0
#define MIDI_MSG_TYPE_PROGRAM_CHANGE    0xC0
#define MIDI_MSG_TYPE_CHAN_PRESSURE     0xD0
#define MIDI_MSG_TYPE_PITCH_WHEEL       0xE0

#define MIDI_BUFFER_RAW_SIZE 16
#define MIDI_BUFFER_MSGS_SIZE 16

#include "stm32f4xx_conf.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"


typedef struct {
    uint8_t msgType;
    uint8_t lowNibble;
    uint8_t dataBytes[MIDI_BASIC_MSG_DATABYTES_MAX];
    uint8_t numberOfDataBytes;
    uint8_t dataByteIndex;
} Midi_basicMsg;

__IO uint8_t midi_rawBuffer[MIDI_BUFFER_RAW_SIZE];
uint8_t midi_rawBufferShadow[MIDI_BUFFER_RAW_SIZE];

uint8_t midi_rawBufferIndex;

Midi_basicMsg midi_msgBuffer[MIDI_BUFFER_MSGS_SIZE];
uint8_t midi_msgBufferIndex;

void midi_initGpio();
void midi_initUSART();
void midi_initDMA();
void midi_initBuffers();
void midi_init();



uint8_t midi_getNumberOfDataBytesForMsgType(uint8_t msgType);

void midi_initMsg(Midi_basicMsg * msg, uint8_t byte);
void midi_addDataByteToMsgIfAble(Midi_basicMsg * msg, uint8_t byte);

void midi_catchUpWithRawBuffer();

int midi_getMsgIfAble(Midi_basicMsg * msg);

#endif /* MIDIIN_H_INCLUDED */
