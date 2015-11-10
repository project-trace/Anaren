/**
 * @ingroup hwExamples
 * @{
 * @file example_read_IR_temperature_sensor.c
 *
 * @brief Uses the I2C interface to read the external IR temperature sensor.
 *
 * @note Due to software averaging / transient reduction, the first four values are not valid
 * @note Requires a larger than default stack size on some processors. Uses ~180B on MSP430.
 *
 * @see http://www.ti.com/lit/gpn/tmp006
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

#include "../HAL/hal.h"
#include "../HAL/hal_TI_TMP006_IR_temperature_sensor.h"
#include "../Common/utilities.h"
#include <stdint.h>

/* calculated temperature value returned by getTemp */
struct TempReading currTemp;

int main( void )
{
    halInit();
    printf("Reading IR Temperature Sensor\r\n");

    /* Initialize the TMP006 */
    tmp006Init();
    printf("Init done\r\n");

    while (1) 
    {
        /* Get current temperature from TMP006 and populate curreTemp structure. Returns raw values */
        tmp006GetTemperature(&currTemp); 

        printf("Raw Data: vObj = %04X, tDie = %04X\r\n", currTemp.vObj, currTemp.tDie);
        float tempDie = (currTemp.tDie >> 2) * 0.03125f;
        int16_t tempDieInt = (int16_t) (tempDie * 100.0f);
        printf("Die Temperature = %d.%03dC\r\n", tempDieInt/100, tempDieInt%100);

        /* Convert degree C to degree F */
        float tempF = ( ((currTemp.temp * (1.8f)) + 32.0f ) );
        int16_t tempFint = (int16_t) (tempF * 100.0f);

        /* Note: we use integers due to space limitations for displaying floats.
        If using a standard stdio library can display float value instead. */
        printf("IR Temperature = %d.%02dC (%d.%02dF)\r\n\r\n", currTemp.tempInt/100, (currTemp.tempInt%100), tempFint/100, tempFint%100);

        delayMs(1000);          
    }

}

/* @} */
