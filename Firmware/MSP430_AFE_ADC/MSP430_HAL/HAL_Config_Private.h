/* --COPYRIGHT--,BSD
 * Copyright (c) 2019, Texas Instruments Incorporated
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
//  MSP430 HAL Configuration
//
//  Static and default configuration of Library
// Texas Instruments, Inc.
#ifndef __CONFIG_PRIVATE_H__
#define __CONFIG_PRIVATE_H__

#include <C:\Users\ASUS\Documents\msp430afe-adc\inc\Config_Common.h>

//*********** GUI Communication ************************************************
//! \brief baudrate of GUI Communication port
//!         i.e. 19200 = 19,200bps
//!
#define HAL_GUICOMM_BAUDRATE              (115200)

//! \brief Select which eUSCI to use with UART
#define GUI_COMM_UART_USCI (0)

#if (GUI_COMM_UART_USCI == 0)
#define UCAnBR0            (UCA0BR0)
#define UCAnBR1            (UCA0BR1)
#define UCAnCTL0           (UCA0CTL0)
#define UCAnCTL1           (UCA0CTL1)
#define UCAnIE             (UCA0IE)
#define UCAnIFG            (UCA0IFG)
#define UCAnIV             (UCA0IV)
#define UCAnMCTL           (UCA0MCTL)
#define UCAnSTAT           (UCA0STAT)
#define UCAnTXBUF          (UCA0TXBUF)
#define UCAnRXBUF          (UCA0RXBUF)

#define USCI_An_VECTOR     (USCIAB0RX_VECTOR)
#define USCI_An_ISR        (USCI_A0_ISR)
#else
#error "eUSCI is not defined. Modify HAL layer"
#endif

#endif //__CONFIG_PRIVATE_H__
