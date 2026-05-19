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

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include <C:\Users\ASUS\Documents\msp430afe-adc\inc\HAL.h>
#include <C:\Users\ASUS\Documents\msp430afe-adc\inc\GUI_mpack.h>
#include <C:\Users\ASUS\Documents\msp430afe-adc\inc\GUIComm.h>
#include <C:\Users\ASUS\Documents\msp430afe-adc\inc\QmathLib.h>
#include "../MSP430_GUI/mpack/mpack.h"


#ifndef INCLUDE_CALLBACKS_H_
#define INCLUDE_CALLBACKS_H_

// #defines for communication protocol commands and flags
#define ADC_A0_PRELOAD  1      // Sets ADC channel A0 preload
#define ADC_A1_PRELOAD  2      // Sets ADC channel A1 preload
#define ADC_A0_GAIN     3      // Sets ADC channel A0 gain
#define ADC_A1_GAIN     4      // Sets ADC channel A1 gain

// Global variables for the callbacks
extern volatile uint8_t  command, adcChannel, adcA0Preload, adcA1Preload, adcA0Gain, adcA1Gain, adcReady;
extern volatile uint16_t adcA0Data, adcA1Data;

// Functions to receive data from GUI
extern void GUICallback_SetA0Preload(mpack_tag_t *tag, mpack_reader_t *reader);
extern void GUICallback_SetA1Preload(mpack_tag_t *tag, mpack_reader_t *reader);
extern void GUICallback_SetA0Gain(mpack_tag_t *tag, mpack_reader_t *reader);
extern void GUICallback_SetA1Gain(mpack_tag_t *tag, mpack_reader_t *reader);

// Functions to read data from mpack
inline extern uint64_t GUICallback_ReadUInt(mpack_tag_t *tag)
{
    return mpack_tag_uint_value(tag);
}

inline extern int64_t GUICallback_ReadInt(mpack_tag_t *tag)
{
    return mpack_tag_int_value(tag);
}

inline extern bool GUICallback_ReadBool(mpack_tag_t *tag)
{
    return mpack_tag_bool_value(tag);
}

inline extern float GUICallback_ReadFloat(mpack_tag_t *tag)
{
    return mpack_tag_float_value(tag);
}

inline extern double GUICallback_ReadDouble(mpack_tag_t *tag)
{
    return mpack_tag_double_value(tag);
}

#endif /* INCLUDE_CALLBACKS_H_ */
