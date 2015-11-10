/**
*  @file hal_fram.h
*
*  @brief public methods for hal_fram.c
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

#ifndef hal_fram_H
#define hal_fram_H

//
//  Common Includes (will be included in all projects that #include this hal file)
//
#include "msp430fr5739.h"
#include <stdint.h>

//
//  METHODS REQUIRED FOR ZIGBEE MODULE AND EXAMPLES
//
void halInit();
void delayMs(unsigned int ms);
int16_t toggleLed(unsigned char led);
int16_t setLed(unsigned char led);
void clearLeds();
void halSpiInitModule();
void spiWrite(unsigned char *bytes, unsigned char numBytes);
void oscInit();

//
//  METHODS NOT REQUIRED FOR ZIGBEE MODULE
//
uint16_t getVcc3();
void halSetAllPinsToInputs(void);

//
// #defines
//
#define HAL_SLEEP()     ( __bis_SR_register(LPM3_bits + GIE))  //works for FR57xx too
#define HAL_WAKEUP()    ( __bic_SR_register_on_exit(LPM3_bits))

#define HAL_ENABLE_INTERRUPTS()         (_EINT())
#define HAL_DISABLE_INTERRUPTS()        (_DINT())

//
//  MACROS REQUIRED FOR ZM
//

#define RADIO_ON()                  (P1OUT |= BIT0)  //ZM Reset Line
#define RADIO_OFF()                 (P1OUT &= ~BIT0)

//  Zigbee Module SPI
#define SPI_SS_SET()                (P1OUT &= ~(BIT2 | BIT3))  //active low, control SS and MRDY
#define SPI_SS_CLEAR()              (P1OUT |= (BIT2 | BIT3))  
#define SRDY_IS_HIGH()              (P2IN & BIT4)
#define SRDY_IS_LOW()               ((~P2IN) & BIT4)

//
//  MISC OTHER DEFINES
//
#define XTAL 8000000L   //Clock speed - used below.
#define TICKS_PER_MS (XTAL / 1000)
//#define TICKS_PER_US (TICKS_PER_MS / 1000)
#define VLO_NOMINAL 10000  //note: this is different from msp430x2xxx devices (they are 12kHz nominal)
#define VLO_MIN (VLO_NOMINAL - (VLO_NOMINAL/4)) //VLO min max = +/- 25% of nominal
#define VLO_MAX (VLO_NOMINAL + (VLO_NOMINAL/4))
#define NUMBER_OF_LEDS      8

#endif
