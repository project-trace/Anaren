/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_write_digital_io.c
 *
 * @brief Resets Module, configures all four of the GPIO pins as outputs, then toggles the GPIO pins
 * once per second. To test, monitor GPIO pins with an oscilloscope.
 *
 * The Module supports four General Purpose Input/Output (GPIO) pins.
 * Pins can be individually configured as:
 * - Inputs
 * - Outputs
 * - Tri-State
 *
 * When configured as an output, the pin can be set, cleared, or toggled.
 * @see example_read_digital_io.c
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

#include "../HAL/hal.h"
#include "../ZM/module.h"
#include "../ZM/module_errors.h"
#include "../ZM/zm_phy_spi.h"
#include <stdint.h>

int main( void )
{
    halInit();
    moduleInit();      
    moduleReset();   
    printf("Configuring Module Digital IO as outputs\r\n");   
    sysGpio(GPIO_SET_DIRECTION , ALL_GPIO_PINS);
    if (sysGpio(GPIO_CLEAR, ALL_GPIO_PINS) != MODULE_SUCCESS)
    {
        printf("sysGpio ERROR\r\n");
    }
    while (1) 
    {
        printf("Setting GPIO pins to high\r\n");
        if (sysGpio(GPIO_SET, ALL_GPIO_PINS) != MODULE_SUCCESS)
        {
            printf("ERROR\r\n"); 
        }        
        toggleLed(1);
        delayMs(1000);    

        printf("Setting GPIO pins to low\r\n");
        if (sysGpio(GPIO_CLEAR, ALL_GPIO_PINS) != MODULE_SUCCESS)
        {
            printf("ERROR\r\n"); 
        }          
        toggleLed(1);
        delayMs(1000); 
    }
}

/* @} */
