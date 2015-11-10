/**
*  @file zm_phy_uart.h
*
*  @brief  public methods for zm_phy_uart.c
*
* $Rev: 1528 $
* $Author: dsmith $
* $Date: 2012-08-30 15:58:08 -0700 (Thu, 30 Aug 2012) $
*
* @section support Support
* Please refer to the wiki at www.anaren.com/air-wiki-zigbee for more information. Additional support
* is available via email at the following addresses:
* - Questions on how to use the product: AIR@anaren.com
* - Feature requests, comments, and improvements:  featurerequests@teslacontrols.com
* - Consulting engagements: sales@teslacontrols.com
*
* @section license License
* Copyright (c) 2012 Tesla Controls. All rights reserved. This Software may only be used with an 
* Anaren A2530E24AZ1, A2530E24CZ1, A2530R24AZ1, or A2530R24CZ1 module. Redistribution and use in 
* source and binary forms, with or without modification, are subject to the Software License 
* Agreement in the file "anaren_eula.txt"
* 
* YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” 
* WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY 
* WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO 
* EVENT SHALL ANAREN MICROWAVE OR TESLA CONTROLS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, 
* STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR 
* INDIRECT DAMAGES OR EXPENSE INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, 
* PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE 
* GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY 
* DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/

#ifndef ZM_PHY_UART_H
#define ZM_PHY_UART_H
#include <stdint.h>
#include "module_errors.h"

moduleResult_t sendMessage();
moduleResult_t receiveAreq(uint8_t intervalMs, int16_t timeoutMs);
moduleResult_t getMessage();
void displayMessageBuffer();
void auxSerialPortHandler(char c);
void zm_phy_init();
uint8_t moduleHasMessageWaiting();

/** The size of the Zigbee Buffer. This is used for both send and receive */
#define ZIGBEE_MODULE_BUFFER_SIZE      0xFF

#define UART_START_OF_FRAME 0xFE
//messageFlags:
#define MESSAGE_BUFFER_IN_USE 0x01  //simple mutex

#define MESSAGE_BUFFER_SIZE 0xFF //largest message is AF_DATA_REQUEST_EXT + SOF + FCS = 0xFF total length
#define UART_MESSAGE_LENGTH_FIELD 0  //SOF is not included in this count
#define MINIMUM_MESSAGE_SIZE  4 //TBD - doesn't include SOF
#define MESSAGE_OVERHEAD 3 //4  //used in computation of message length

#define SRSP_BUFFER_SIZE        20
#define SRSP_HEADER_SIZE        3

//SRSP MSB is 0x40 greater than SREQ MSB
#define SRSP_OFFSET             0x40

#define SRSP_PAYLOAD_START      3
#define SRSP_LENGTH_FIELD       0  
#define SRSP_CMD_LSB_FIELD      2
#define SRSP_CMD_MSB_FIELD      1

#endif
