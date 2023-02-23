#include "main.h"

#ifndef SOFTUART_H_
#define SOFTUART_H_

#define Number_Of_SoftUarts 1 // Max 8

#define SoftUartRxBufferSize 127

#define SoftUart_DATA_LEN 8 // Max 8 Bit

typedef struct {
  uint8_t RxBitShift, RxBitCounter;
  uint8_t LastRxIndex;
  uint8_t RxIndex;
  uint8_t Buffer[256];
  GPIO_TypeDef *RxPort;
  uint16_t RxPin;
  uint8_t RxTimingFlag;
  uint8_t RxBitOffset;
} SoftUart_S;

void SoftUartHandler();

uint8_t SoftUartRxAlavailable();
void SoftUartInit(GPIO_TypeDef *RxPort, uint16_t RxPin);
uint8_t SoftUartReadRxBuffer(uint8_t *Buffer);

#endif
