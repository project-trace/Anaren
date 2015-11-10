/**
* @ingroup hal
* @{ 
* @file hal_TI_TMP006_IR_temperature_sensor.h
*
* @brief Hardware Abstraction Layer (HAL) file for TMP006
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

#ifndef hal_TI_TMP006_IR_temperature_sensor_H
#define hal_TI_TMP006_IR_temperature_sensor_H

#include <stdint.h>

/** Temperature reading structure */
struct TempReading
{
    /** Calculated temperature reading */
    float temp;

    /** Stores the current object voltage */
    int16_t vObj;

    /** Stores the current ambient temperature */
    int16_t tDie;
    
    /** The temperature, stored as an integer */
    int16_t tempInt;
};

void tmp006Init();
void tmp006GetTemperature(struct TempReading* tempRead);

/*TMP006 I2C Address. Note: may be hardware dependent. */
#define TMP006_I2C_ADDRESS 0x40

/** TMP006 object voltage register pointer */
#define TMP006_P_VOBJ       0x00
/** TMP006 ambient temperature register pointer */
#define TMP006_P_TABT       0x01
/** TMP006 configuration register pointer */
#define TMP006_P_WRITE_REG  0x02
/** TMP006 manufacturer ID register pointer */
#define TMP006_P_MAN_ID     0xFE
/** TMP006 device ID register pointer */
#define TMP006_P_DEVICE_ID  0xFF

/** TMP006 Configuration Register Bit - Reset */
#define TMP006_RST          0x8000
/** TMP006 Configuration Register Bit - Power Down */
#define TMP006_POWER_DOWN   0x0000
/** TMP006 Configuration Register Bit - Power Up */
#define TMP006_POWER_UP     0x7000

// Conversion Rates - note: the faster the sample rate, the more error
/** Conversion rate of 4 per sec; 1 avg */
#define TMP006_CR_4         0x0000
/** Conversion rate of 2 per sec; 2 avg */
#define TMP006_CR_2         0x0200
/** Conversion rate of 1 per sec; 4 avg */
#define TMP006_CR_1         0x0400
/** Conversion rate of 0.5 per sec; 8 avg */
#define TMP006_CR_0_5       0x0600
/** Conversion rate of 0.25 per sec; 16 avg */
#define TMP006_CR_0_25      0x0800

#define TMP006_EN           0x0100
#define TMP006_DRDY         0x0080



#endif

/* @} */
