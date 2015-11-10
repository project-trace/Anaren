/**
*  @file hal_mdb1.h
*
*  @brief  public methods for hal_mdb1.c
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

#ifndef MDB1_H
#define MDB1_H

#include "msp430x24x.h"
#include <stdio.h>
#include "../Common/utilities.h"

#define HAL_SLEEP()     ( __bis_SR_register(LPM3_bits + GIE))
#define HAL_WAKEUP()    ( __bic_SR_register_on_exit(LPM3_bits))

#define NUMBER_OF_LEDS  6 //not including status LEDs
#define STATUS_LED_RED BIT6
#define STATUS_LED_GREEN BIT7
#define STATUS_LED_YELLOW (BIT6+BIT7)
#define STATUS_LED_OFF  0
#define ALL_LEDS_OFF 0x3F 

//options for halConfigureSrdyInterrrupt()
#define SRDY_INTERRUPT_FALLING_EDGE 0x01
#define SRDY_INTERRUPT_RISING_EDGE  0x00

void halInit();
void toggleLed(unsigned char whichLed);
signed int setLed(unsigned char led);
void clearLeds();
void halSpiInitModule();
void spiWrite(unsigned char *bytes, unsigned char numBytes);
void setModuleInterfaceToInputs(void);
signed int initTimer(unsigned char seconds, unsigned char wakeOnTimer);
void delayMs(unsigned int ms);

void halEnableAuxFlowControl();

//NOT REQUIRED FOR MODULE LIBRARY:
void oscInit();
unsigned char getSwitches();
unsigned char getButtons();
signed char setStatusLed(unsigned char color);
void setButtonLeds(unsigned char buttonId);
void clearButtonLeds();
int putcharAux(int c);
//void stopTimerA();
//void delayUs(unsigned char hundredsOfMicroSeconds);
unsigned int getVcc3();
unsigned int getVddUnregulated();

//REQUIRED FOR MODULE LIBRARY:
#define RADIO_ON()                  (P1OUT |= BIT2)
#define RADIO_OFF()                 (P1OUT &= ~BIT2)

#define SPI_SS_SET()                (P5OUT &= ~BIT0)  //active low
#define SPI_SS_CLEAR()              (P5OUT |= BIT0)  
#define SRDY_IS_HIGH()              (P1IN & BIT3)
#define SRDY_IS_LOW()               ((~P1IN) & BIT3)

#define HAL_ENABLE_INTERRUPTS()         (_EINT())
#define HAL_DISABLE_INTERRUPTS()        (_DINT())

//NOT REQUIRED FOR MODULE LIBRARY:
#define DEBUG_ON()                  (P6OUT |= BIT6)
#define DEBUG_OFF()                 (P6OUT &= BIT6)
#define DEBUG_TOGGLE()              (P6OUT ^= BIT6)

#define GET_MCLK_FREQ()             8000000L 
#define HAL_GET_SMCLK()             4000000L  //used in hal_flash_write.c
#define XTAL 8000000L

//REQUIRED FOR MODULE UART INTERFACE
#define RTS_OFF()                   (P1OUT |= BIT0)  //Stop the module from sending bytes so we can process them
#define RTS_ON()                    (P1OUT &= ~BIT0) //Module can resume sending bytes


//REQUIRED FOR HAL UTILITIES
#define SET_LED0_PIN_DIRECTION() (P4DIR = BIT0)

#define MASTER_BUTTON_0 0x00
#define MASTER_BUTTON_1 0x01
#define MASTER_BUTTON_2 0x02
#define MASTER_BUTTON_3 0x03
#define MASTER_BUTTON_4 0x04

#define BUTTON_LEDS_OFF 0xFF

#define DEBUG_CONSOLE_INIT(baud) (halUartInit(USCIA0, baud))
#define AUX_SERIAL_PORT_INIT(baud) (halUartInit(USCIA1, baud))

//Timer defines
#define TIMER_A                 0
#define TIMER_B                 1
#define NO_WAKEUP               0
#define WAKEUP_AFTER_TIMER      1
#define WAKEUP_AFTER_SRDY       4  //???
#define HAL_STOP_TIMER_A()      TACTL = MC_0;
#define TIMER_MAX_SECONDS       4

// Bit-bang UART defines
// Note: if using bit-bang UART, be sure to also modify the ISR in Port1 or Port2 for bitBangSerialIsr
/*
#define BIT_BANG_RX0_PORT                   P2IN
#define BIT_BANG_RX0_BIT                    BIT6 //BIT5
#define DISABLE_BIT_BANG_RX0_INTERRUPT()    (P2IE &= ~BIT6) //BIT5)
#define ENABLE_BIT_BANG_RX0_INTERRUPT()     (P2IE |= BIT6)   //BIT5)
*/

//Note: Signal is inverted through opto!
//EXP IN
#define BIT_BANG_TX0_PORT                   P2OUT
#define BIT_BANG_TX0_BIT                    BIT6  //BIT7
#define BIT_BANG_RX0_PORT                   P1IN
#define BIT_BANG_RX0_BIT                    BIT4 //BIT5
#define DISABLE_BIT_BANG_RX0_INTERRUPT()    (P1IE &= ~BIT4) //BIT5
#define ENABLE_BIT_BANG_RX0_INTERRUPT()     (P1IE |= BIT4)   //BIT5

//EXP OUT/THRU
#define BIT_BANG_TX1_PORT                   P2OUT
#define BIT_BANG_TX1_BIT                    BIT7
#define BIT_BANG_RX1_PORT                   P1IN
#define BIT_BANG_RX1_BIT                    BIT5
#define DISABLE_BIT_BANG_RX1_INTERRUPT()    (P1IE &= ~BIT5)
#define ENABLE_BIT_BANG_RX1_INTERRUPT()     (P1IE |= BIT5)


#define TOGGLE_DEBUG_PIN()                  (P4OUT ^= BIT5)  //debugging only

/* Required for hal_gruner_707_latching_relay
NOTE: this only works for Rev B. Rev A is reversed on/off! */
#define RELAY_0_0FF_PORT    P5OUT
#define RELAY_0_0FF_PIN     BIT5

#define RELAY_0_ON_PORT     P5OUT
#define RELAY_0_ON_PIN      BIT4

#define RELAY_1_0FF_PORT    P5OUT
#define RELAY_1_0FF_PIN     BIT7

#define RELAY_1_ON_PORT     P5OUT
#define RELAY_1_ON_PIN      BIT6

void halSetAllPinsToInputs();

#endif
