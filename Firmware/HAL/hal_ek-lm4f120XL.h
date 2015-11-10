/*
* @file hal_ek-lm4f120XL.h
*
* @brief public methods for hal_ek-lm4f120XL.c Adapted from hal_gw1.h.
*
* $Rev: 1568 $
* $Author: dsmith $
* $Date: 2012-09-24 17:16:29 -0700 (Mon, 24 Sep 2012) $
*
* @section support Support
* Please refer to the wiki at http://teslacontrols.com/mw/ for more information. Additional support
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
* 
*/

#ifndef HAL_EK_LM4F120XL_H
#define HAL_EK_LM4F120XL_H

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include <stdint.h>

/** Redirect printf to the Stellaris UARTprintf() implementation. Requires including utils/uartstdio.c */
#define printf UARTprintf

//
//  CORE METHODS REQUIRED FOR ZIGBEE MODULE INTERFACE
//
void halInit();
void delayMs(uint16_t ms);
void halSpiInitModule();
void spiWrite(uint8_t *bytes, uint8_t numBytes);
int16_t toggleLed(uint8_t whichLed);
int16_t setLed(uint8_t led);
int16_t clearLed(uint8_t led);
void clearLeds();

#define ON_NETWORK_LED          2
#define NETWORK_FAILURE_LED     1
#define SEND_MESSAGE_LED        3

//
//  OTHER
//
void halSetWakeupFlags(uint16_t wakeupFlagsToSet);
void halRgbLedPwmInit();
void halRgbSetLeds(uint8_t red, uint8_t blue, uint8_t green);
// For RGB LED interface consistency:
//RGB LED:
#define RGB_LED_PWM_OFF                	0x00
#define RGB_LED_PWM_PERIOD              0xFF

uint8_t halUartBusy();
uint16_t getCurrentSensor();
uint8_t buttonIsPressed(uint8_t button);



/** Get the processor clock frequency */
#define GET_MCLK_FREQ() (SysCtlClockGet())
#define TICKS_PER_MS	25000			// used in FAST_PROCESSOR option of zm_phy
#define TICKS_PER_US (TICKS_PER_MS / 1000)
//#define WAIT_WHILE_SPI_BUSY()  while ((HWREG(SSI0_BASE + SSI_O_SR)) & SSI_SR_BSY)   //wait while busy

// Buttons
#define ANY_BUTTON          0xFF
#define BUTTON_0            0
#define BUTTON_1            1
#define BUTTON_2            2

/* Peripherals:
LED0 - Red LED on BoosterPack, PB5
LED1 - Blue of RGB LED on BoosterPack, PB2
Can also use the Green LED on BoosterPack, but it's shared with MISO
Note: For pretty colors we're using the RGB LED on the Stellaris LaunchPad, not the one on the BoosterPack,
since the one on the LaunchPad is controlled by hardware PWM.
*/

//Module RST = PE0
//Module MRDY = CS = PA5
//Module SRDY = PA7
//PRECONDITIONS: GPIO PORTS HAVE BEEN ENABLED & PINS WERE CONFIGURED: RST, MRDY as outputs; SRDY as input
#define RADIO_ON()                  ( GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0) )
#define RADIO_OFF()                 ( GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0) )
#define SPI_SS_SET()                ( GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0) ) 	//MRDY tied to CS in hardware?
#define SPI_SS_CLEAR()              ( GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PIN_5) )
#define SRDY_IS_HIGH()              (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7) & GPIO_PIN_7)
#define SRDY_IS_LOW()               (~(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_7)) & GPIO_PIN_7)

#define HAL_ENABLE_INTERRUPTS()		IntMasterEnable()
#define HAL_DISABLE_INTERRUPTS()	IntMasterDisable()

#define DEBUG_ON()
#define DEBUG_OFF()

#define HAL_SLEEP()     ( SysCtlSleep() )
//#define HAL_WAKEUP()

int16_t calibrateVlo();
int16_t initTimer(uint8_t seconds);
#define NO_WAKEUP                   0
#define WAKEUP_AFTER_TIMER          1
#define WAKEUP_AFTER_BUTTON         2


//#define XTAL                        25000000L


#endif
