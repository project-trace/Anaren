/**
* @ingroup hal
* @{ 
* @file hal_TI_TMP006_IR_temperature_sensor.h
*
* @brief Hardware Abstraction Layer (HAL) file for TMP006
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
* Copyright (c) 2012 Anaren Microwave. All rights reserved. This Software may only be used with an 
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

#ifndef hal_Microchip_24xxxx_eeprom_H
#define hal_Microchip_24xxxx_eeprom_H

#include <stdint.h>

/*EEPROM I2C Address. Note: may be hardware dependent. */
#define EEPROM_I2C_ADDRESS      (0x50)

#define EEPROM_MAX_ADDRESS      (0x4000) // For 16kB EEPROM

#define EEPROM_PAGE_SIZE        (64)

int16_t eepromRandomRead(uint16_t address);
int16_t eepromByteWrite(uint16_t address, uint8_t value);
int16_t eepromSequentialRead(uint16_t address, uint8_t* data, uint8_t numBytes);
void eepromInit();  
uint8_t eepromReadCurrentAddress();
int16_t eepromPageWrite(uint16_t address, uint8_t* data, uint8_t numBytes);
uint8_t eepromBusy();

#endif

/* @} */
