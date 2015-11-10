/**
* @ingroup hal
* @{ 
* @file hal_bit_bang_i2c.h
*
* @brief Implements an I2C interface using software
*
* Uses the Hardware Abstraction Layer (hal).
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

#include "hal_launchpad.h"

#ifndef HAL_BIT_BANG_I2C_H
#define HAL_BIT_BANG_I2C_H

/** Note: on LaunchPad, P2.4 is SCL, P2.3 is SDA. */

void i2cInitPins();

/* Port registers */
#define I2C_SDA_DIR_PORT    P2DIR
#define I2C_SCL_DIR_PORT    P2DIR
#define I2C_SDA_IN_PORT     P2IN
#define I2C_SDA_OUT_PORT    P2OUT
#define I2C_SCL_OUT_PORT    P2OUT

/* Pins for each I2C signal, correspond to port registers. */
#define I2C_MASTER_SCL BIT4
#define I2C_MASTER_SDA BIT3

uint8_t i2cAddressTest(uint8_t addressToTest);
void i2cAddressSearch(void);
uint8_t i2cWrite(uint8_t* bytes, uint8_t numBytes);
int8_t i2cInit(uint8_t i2cAddress);

uint8_t i2cRead(uint8_t* bytes, uint8_t numBytes);
uint8_t i2cReadOneByte(uint8_t* value);
void displayI2cError(unsigned long i2cErr);

#endif //HAL_BIT_BANG_I2C_H

/* @} */

