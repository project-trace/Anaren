/**
*  @file hal_launchpad.h
*
*  @brief public methods for hal_launchpad.c
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

#ifndef hal_launchpad_H
#define hal_launchpad_H

//
//  Common Includes (will be included in all projects that #include this hal file)
//
#include "msp430g2553.h"        // Processor Definition File

#ifdef USE_STDIO                // On some projects, don't use the limited Common/printf; use the full version
#include <stdio.h>
#else
#include "../Common/printf.h"   // Otherwise use the size-optimized version
#endif

#include <stdint.h>             // Standard integers (uint8_t, int32_t, etc.)

//
//  CORE METHODS REQUIRED FOR ZIGBEE MODULE AND EXAMPLES
//
void halInit();
void delayMs(uint16_t delay);
int16_t toggleLed(uint8_t led);
int16_t setLed(uint8_t led);
int16_t clearLed(uint8_t led);
void clearLeds();

#define ON_NETWORK_LED          2
#define NETWORK_FAILURE_LED     1
#define SEND_MESSAGE_LED        3

void halSpiInitModule();
void spiWrite(uint8_t *bytes, uint8_t numBytes);
int16_t calibrateVlo();
int16_t initTimer(uint8_t seconds);
void halSetWakeupFlags(uint16_t wakeupFlagsToSet);

//
//  METHODS NOT REQUIRED FOR ZIGBEE MODULE
//
uint16_t getVcc3();
void halSetAllPinsToInputs(void);
void halClearWakeupFlags(uint16_t wakeupFlagsToClear);
uint8_t buttonIsPressed(uint8_t button);

void halRgbLedPwmInit();
void halRgbSetLeds(uint8_t red, uint8_t blue, uint8_t green);
void halRgbLedTest();

uint8_t halUartBusy();

uint16_t getCurrentSensor();

// Buttons
#define ANY_BUTTON          0xFF
#define BUTTON_0            0


//RGB LED:
#define RGB_LED_PWM_OFF                0x00   // For API methods
#define RGB_LED_PWM_PERIOD              0xFF


//
// #defines
//
#define HAL_SLEEP()     ( __bis_SR_register(LPM3_bits + GIE))
#define HAL_WAKEUP()    ( __bic_SR_register_on_exit(LPM3_bits))

#define ENABLE_SRDY_INTERRUPT()     (P2IE |= BIT2)
#define DISABLE_SRDY_INTERRUPT()    (P2IE &= ~BIT2)

#define HAL_ENABLE_INTERRUPTS()         (_EINT())
#define HAL_DISABLE_INTERRUPTS()        (_DINT())

//
//  MACROS REQUIRED FOR ZM
//
#define RADIO_ON()                  (P2OUT |= BIT7)  //ZM Reset Line
#define RADIO_OFF()                 (P2OUT &= ~BIT7)
//#define SET_LED0_PIN_DIRECTION()    (P1DIR = BIT0)

//  Zigbee Module SPI
#define SPI_SS_SET()                (P2OUT &= ~BIT0)  //active low, control SS and MRDY
#define SPI_SS_CLEAR()              (P2OUT |= BIT0)  
#define SRDY_IS_HIGH()              (P2IN & BIT2)
#define SRDY_IS_LOW()               ((~P2IN) & BIT2)

#define NO_WAKEUP                   0
#define WAKEUP_AFTER_TIMER          1
#define WAKEUP_AFTER_BUTTON         2
#define WAKEUP_AFTER_SRDY           4

//
//  MISC OTHER DEFINES
//
#define XTAL 8000000L   //Clock speed - used below.
#define TICKS_PER_MS (XTAL / 1000)
#define TICKS_PER_US (TICKS_PER_MS / 1000)
#define VLO_NOMINAL 12000
#define VLO_MIN (VLO_NOMINAL - (VLO_NOMINAL/4)) //VLO min max = +/- 25% of nominal
#define VLO_MAX (VLO_NOMINAL + (VLO_NOMINAL/4))
#define NUMBER_OF_LEDS      2

// RGB LEDs:
#define RGB_LED_GREEN           0
#define RGB_LED_BLUE            1
#define RGB_LED_RED             2

//TMP121 Temperature Sensor Interface:
#define DESELECT_TEMPERATURE_SENSOR()     (P2OUT |= BIT4)
#define SELECT_TEMPERATURE_SENSOR()       (P2OUT &= ~BIT4)



#endif
