
/**
* @file hal_AMS_TCS3414_color_sensor.h
*
* @brief Hardware Abstraction Layer (HAL) file for AMS TCS3414 Color Sensor
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

#ifndef hal_AMS_TCS3414_color_sensor_H
#define hal_AMS_TCS3414_color_sensor_H

#include <stdint.h>

void colorSensorInit();
struct ColorReading getColor(void);
uint16_t convertRGBToCCT(uint16_t red, uint16_t green, uint16_t blue);

/** The I2C address of the TCS3414 color sensor. Note that this is the "correct" I2C address and does
not include read vs. write; that is set by the I2C driver. */
#define COLOR_SENSOR_I2C_ADDRESS                  (0x39)

#define TCS3414_DELAY_MS                          13

/** SMBus interface requires repeating the I2C address */
#define TCS3414_READ_ADDRESS                      (0x73)

/* The following are from the TCS3414 datasheet */

#define TCS3414_COMMAND_BIT                       (0x80)    // Must be 1
#define TCS3414_WORD_BIT                          (0x20)    // 1 = read/write word (rather than byte)

#define TCS3414_REGISTER_CONTROL                  (0x00)
#define TCS3414_REGISTER_TIMING                   (0x01)
#define TCS3414_REGISTER_INTERRUPT                (0x02)
#define TCS3414_REGISTER_INTSOURCE                (0x03)
#define TCS3414_REGISTER_PARTNO_REVID             (0x04)
#define TCS3414_REGISTER_GAIN                     (0x07)
#define TCS3414_REGISTER_LOWTHRESHOLD_LOWBYTE     (0x08)
#define TCS3414_REGISTER_LOWTHRESHOLD_HIGHBYTE    (0x09)
#define TCS3414_REGISTER_HIGHTHRESHOLD_LOWBYTE    (0x0A)
#define TCS3414_REGISTER_HIGHTHRESHOLD_HIGHBYTE   (0x0B)
#define TCS3414_REGISTER_GREENLOW                 (0x10)
#define TCS3414_REGISTER_GREENHIGH                (0x11)
#define TCS3414_REGISTER_REDLOW                   (0x12)
#define TCS3414_REGISTER_REDHIGH                  (0x13)
#define TCS3414_REGISTER_BLUELOW                  (0x14)
#define TCS3414_REGISTER_BLUEHIGH                 (0x15)
#define TCS3414_REGISTER_CLEARLOW                 (0x16)
#define TCS3414_REGISTER_CLEARHIGH                (0x17)

//Control Register
#define TCS3414_CONTROL_POWERON                   (0x01)
#define TCS3414_CONTROL_POWEROFF                  (0x00)
#define TCS3414_CONTROL_ADC_EN                    (0x02)    // Integration mode and time/counter should be written before ADC_EN is asserted
#define TCS3414_CONTROL_ADC_VALID                 (0x10)    // Integration mode and time/counter should be written before ADC_EN is asserted

//Gain Register
#define TCS3414_GAIN_GAINMASK                     (0x30)
#define TCS3414_GAIN_PRESCALARMASK                (0x07)
#define TCS3414_GAIN_1X                           (0x00)
#define TCS3414_GAIN_4X                           (0x10)
#define TCS3414_GAIN_16X                          (0x20)
#define TCS3414_GAIN_64X                          (0x30)

//Timing Register
#define TCS3414_INTEGRATION_TIME_MASK              (0x03)
#define TCS3414_INTEGRATION_TIME_12MS              (0x00)
#define TCS3414_INTEGRATION_TIME_100MS              (0x01)
#define TCS3414_INTEGRATION_TIME_400MS              (0x02)


/** Color reading structure. Holds all four sensor readings. */
struct ColorReading
{
    /** Value read from the red sensor */
    uint16_t red;
    /** Value read from the green sensor */    
    uint16_t green;
    /** Value read from the blue sensor */    
    uint16_t blue;
    /** Value read from the clear sensor */    
    uint16_t clear;
};


#endif

/* @} */
