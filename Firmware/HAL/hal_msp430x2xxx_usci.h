/**
*  @file hal_msp430x2xxx_usci.h
*
*  @brief  public methods for hal_msp430F2xxx.c
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

#ifndef HAL_MSP430X2XXX_USCI_H
#define HAL_MSP430X2XXX_USCI_H
#ifdef MDB1
#include "msp430x24x.h"
#elif defined MDB2
#error "MDB2 is not supported in this release"
#include "msp430x22x4.h"
#elif defined LAUNCHPAD
#include "msp430g2553.h"
#else
#error "To use the USCI library You must define a board configuration: MDB1, MDB2, or LAUNCHPAD. In IAR this is done in Project Options : C/C++ Compiler : Preprocessor : Defined Symbols. In CCS this is done in Project Properties : Build : MSP430 Compiler : Advanced Options : Predefined Symbols."
#endif
#include <stdint.h>

//#include "../Common/utilities.h"


//UART:
signed int halUartInit(unsigned char port, unsigned char baudRate);
signed int halUartCharPut(unsigned char port, unsigned char data);

//SPI:
signed int halSpiInit(unsigned char port, unsigned char baudRate);
signed int halSpiWrite(unsigned char port, unsigned char *bytes, unsigned char numBytes);


//baud rate options for UART init
#define BAUD_RATE_9600      0x00
#define BAUD_RATE_19200     0x01
#define BAUD_RATE_38400     0x02
#define BAUD_RATE_115200    0x03

//baud rate options for SPI init
#define BAUD_RATE_500KHZ      0x10
#define BAUD_RATE_1MHZ      0x11
#define BAUD_RATE_2MHZ      0x12

//port options for UART/SPI init
#define ERROR_HAL_INVALID_USCI_PORT -1
#define ERROR_HAL_INVALID_BAUD_RATE -2
#define USCIA0          0x00
#define USCIA1          0x01
#define USCIB0          0x02
#define USCIB1          0x03



#endif
