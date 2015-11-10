/**
*  @file hal_mdb2.h
*
*  @brief public methods for hal_mdb2.c
*
* $Rev: 1642 $
* $Author: dsmith $
* $Date: 2012-10-31 11:17:04 -0700 (Wed, 31 Oct 2012) $
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

#ifndef hal_MDB2_H
#define hal_MDB2_H

//
//  Common Includes (will be included in all projects that #include this hal file)
//
#include "hal_mdb2.h"
#include "msp430x22x4.h"


//
//  METHODS REQUIRED FOR ZIGBEE MODULE AND EXAMPLES
//
void halInit();
void delayMs(unsigned int ms);
void delayUs(unsigned int us);
signed int toggleLed(unsigned char led);
signed int setLed(unsigned char led);
void clearLeds();
void halSpiInitModule();
void spiWrite(unsigned char *bytes, unsigned char numBytes);
signed int calibrateVlo();
signed int initTimer(unsigned char seconds);
void oscInit();

//
//  METHODS NOT REQUIRED FOR MODULE
//
unsigned int getVcc3();

void halSetAllPinsToInputs(void);
signed int halConfigureSrdyInterrrupt(unsigned char flags);

//
// #defines
//
#define HAL_SLEEP()     ( __bis_SR_register(LPM3_bits + GIE))
#define HAL_WAKEUP()    ( __bic_SR_register_on_exit(LPM3_bits))

#define ENABLE_SRDY_INTERRUPT()     (P2IE |= BIT6)
#define DISABLE_SRDY_INTERRUPT()    (P2IE &= ~BIT6)

#define HAL_ENABLE_INTERRUPTS()         (_EINT())
#define HAL_DISABLE_INTERRUPTS()        (_DINT())

//options for halConfigureSrdyInterrrupt()
#define SRDY_INTERRUPT_FALLING_EDGE 0x01
#define SRDY_INTERRUPT_RISING_EDGE  0x00

//
//  MACROS REQUIRED FOR ZM
//
#define RADIO_ON()                  (P3OUT |= BIT7)  //ZM Reset Line
#define RADIO_OFF()                 (P3OUT &= ~BIT7)
#define SET_LED0_PIN_DIRECTION()    (P1DIR = BIT0)

//  Zigbee Module SPI
#define SPI_SS_SET()                (P3OUT &= ~(BIT0 | BIT6))  //active low, control SS and MRDY
#define SPI_SS_CLEAR()              (P3OUT |= (BIT0 | BIT6))  
#define SRDY_IS_HIGH()              (P2IN & BIT6)
#define SRDY_IS_LOW()               ((~P2IN) & BIT6)

#define DEBUG_ON()                  (P4OUT |= BIT4)
#define DEBUG_OFF()                 (P4OUT &= BIT4)

#define NO_WAKEUP                   0
#define WAKEUP_AFTER_TIMER          1
#define WAKEUP_AFTER_BUTTON         2
#define WAKEUP_AFTER_SRDY           4

//
//  MISC OTHER DEFINES
//
#define XTAL 8000000L   //Clock speed - used below.
#define TICKS_PER_MS (XTAL / 1000)
//#define TICKS_PER_US (TICKS_PER_MS / 1000)
#define VLO_NOMINAL 12000
#define VLO_MIN (VLO_NOMINAL - (VLO_NOMINAL/4)) //VLO min max = +/- 25% of nominal
#define VLO_MAX (VLO_NOMINAL + (VLO_NOMINAL/4))
#define NUMBER_OF_LEDS      2
//#define GET_MCLK_FREQ()     8000000L    



#endif