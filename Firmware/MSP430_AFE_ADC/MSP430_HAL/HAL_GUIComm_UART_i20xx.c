/* --COPYRIGHT--,BSD
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//        GUI HAL for MSP430i20xx using UART
//
// Driver to send and receive data from GUI using i20xx UART
// Texas Instruments, Inc.
// *****************************************************************************
#include "HAL_Config_Private.h"
#include <C:\Users\ASUS\Documents\msp430afe-adc\inc\HAL.h>

tGUICommRXCharCallback RxByteISRCallback;

void HAL_GUI_Init(tGUICommRXCharCallback RxByteCallback)
{
    // Store callback for ISR RX Byte
    RxByteISRCallback = RxByteCallback;

    // Configure UART
    U0CTL |= SWRST;            // Hold eUSCI in reset

#if (HAL_GUICOMM_BAUDRATE == 115200)

    U0CTL |= SSEL0;      // SMCLK
    U0BR0 = 10;                               // 1MHz 115200
    U0BR1 = 0x00;                             // 1MHz 115200
    U0MCTL = 0x00;                            // 1MHz 115200 
    
#else
#error "Define UART baudrate registers based on desired frequency"
#endif

    U0CTL &= ~SWRST;           // Release from reset
    IE1 |= URXIE0;               // Enable RX interrupt
}

void HAL_GUI_TransmitCharBlocking(char character)
{
    // Transmit Character
    TXBUF0 = character;

}

// EUSCI interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USART0RX_VECTOR
__interrupt void USART0_RX (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USART0RX_VECTOR))) USART0_RX (void)
#else
#error "Compiler not supported!"
#endif
;
