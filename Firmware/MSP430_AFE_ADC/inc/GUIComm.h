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
//         Generic GUI Demo using MSP430
//
// API to communicate with GUI and abstract transport layer protocol
// Texas Instruments, Inc.
// ******************************************************************************

#ifndef INCLUDE_GUICOMM_H_
#define INCLUDE_GUICOMM_H_

//! ***** Functions to send data to GUI *****

//! \brief Sends a 32-bit integer.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendInt(char *cmd, uint8_t cmdLength, uint32_t val);

//! \brief Sends a 32-bit unsigned integer.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendUInt(char *cmd,  uint8_t cmdLength, uint32_t val);

//! \brief Sends an uint8_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendUInt8(char *cmd, uint8_t cmdLength, uint8_t val);

//! \brief Sends an int8_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendInt8(char *cmd, uint8_t cmdLength, int8_t val);

//! \brief Sends an uint16_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendUInt16(char *cmd, uint8_t cmdLength, uint16_t val);

//! \brief Sends an int16_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendInt16(char *cmd, uint8_t cmdLength, int16_t val);

//! \brief Sends an uint32_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendUInt32(char *cmd, uint8_t cmdLength, uint32_t val);

//! \brief Sends an int32_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendInt32(char *cmd, uint8_t cmdLength, int32_t val);

//! \brief Sends an uint32_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendUInt64(char *cmd, uint8_t cmdLength, uint64_t val);

//! \brief Sends an int32_t value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendInt64(char *cmd, uint8_t cmdLength, int64_t val);

//! \brief Sends a bool value to the GUI.
//!
//! \return 0 if success, -1 if failure
extern int GUIComm_sendBool(char *cmd, uint8_t cmdLength, bool val);



#endif /* INCLUDE_GUICOMM_H_ */
