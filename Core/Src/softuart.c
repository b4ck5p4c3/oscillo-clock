#include "softuart.h"

// Some internal define
#define SoftUart_DATA_LEN_C1 (SoftUart_DATA_LEN + 1)
#define SoftUart_DATA_LEN_C2 (SoftUart_DATA_LEN + 2)

// All Soft Uart Config and State
SoftUart_S SUart;

// For timing division
__IO uint8_t SU_Timer=0;

// Read RX single Pin Value
GPIO_PinState SoftUartGpioReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

// Initial Soft Uart
void SoftUartInit(GPIO_TypeDef *RxPort, uint16_t RxPin) {
    SUart.RxBitCounter = 0;
    SUart.RxBitShift = 0;
    SUart.RxIndex = 0;
    SUart.LastRxIndex = 0;

    for (uint16_t i = 0; i < 256; i++) {
      SUart.Buffer[i] = 0;
    }

    SUart.RxPort = RxPort;
    SUart.RxPin = RxPin;

    SUart.RxTimingFlag = 0;
    SUart.RxBitOffset = 0;
}

// Read Size of Received Data in buffer
uint8_t SoftUartRxAlavailable() {
    return (0u + SUart.RxIndex + 256 - SUart.LastRxIndex) & 0xFF;
}

// Move Received Data to Another Buffer
uint8_t SoftUartReadRxBuffer(uint8_t *Buffer) {
    uint8_t i = SUart.LastRxIndex;
    uint8_t max = SUart.RxIndex;
    uint8_t *lastBuffer = Buffer;
    for (; i != max; i++) {
        *Buffer++ = SUart.Buffer[i];
    }
    SUart.LastRxIndex = max;
    return Buffer - lastBuffer;
}

// Soft Uart Receive Data Process
void SoftUartRxDataBitProcess(uint8_t B0_1) {
    // Start
    if (SUart.RxBitCounter == 0) {
        // Start Bit is 0
        if (B0_1) {
          return;
        }

        SUart.RxBitShift = 0;
        SUart.RxBitCounter++;
        SUart.Buffer[SUart.RxIndex] = 0;
    }
    // Data
    else if (SUart.RxBitCounter < SoftUart_DATA_LEN_C1) {
        SUart.Buffer[SUart.RxIndex] |= ((B0_1 & 0x01) << SUart.RxBitShift);
        SUart.RxBitCounter++;
        SUart.RxBitShift++;
    }
    // Stop and Complete
    else if (SUart.RxBitCounter == SoftUart_DATA_LEN_C1) {
        SUart.RxBitCounter = 0;
        SUart.RxTimingFlag = 0;

        //Stop Bit must be 1
        if (B0_1) {
            SUart.RxIndex++;
        }
        // if not : ERROR -> Overwrite data
    }
}

// Capture RX and Get BitOffset
uint8_t SoftUartScanRxPorts() {
    uint8_t Bit;

    // Read RX GPIO Value
    Bit = SoftUartGpioReadPin(SUart.RxPort, SUart.RxPin);

    // Starting conditions
    if (!SUart.RxBitCounter && !SUart.RxTimingFlag && !Bit) {
        // Save RX Bit Offset
        // Calculate middle position of data pulse
        SUart.RxBitOffset = ((SU_Timer + 2) % 5);

        // Timing Offset is Set
        SUart.RxTimingFlag = 1;
    }

    return Bit & 0x01;
}

// SoftUartHandler must call in interrupt every 0.2*(1/BR)
// if BR=9600 then 0.2*(1/9600)=20.8333333 uS
void SoftUartHandler() {
    uint8_t SU_DBuffer;

    // Capture RX and Get BitOffset
    SU_DBuffer = SoftUartScanRxPorts();

    if (SUart.RxBitOffset == SU_Timer) {
        SoftUartRxDataBitProcess(SU_DBuffer);
    }

    // Timing process
    SU_Timer++;
    if (SU_Timer >= 5) {
      SU_Timer=0;
    }
}
