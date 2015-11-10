/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_measure_module_current.c
 *
 * @brief Simple utility to demonstrate how to measure module current using onboard current sense
 * amplifier. To generate sufficient current to measure we turn the module on and off using its RF
 * Test functionality.
 *
 * @note the current sense amplifier cannot be used to measure module sleep current; you'll need a
 * precision micro-amperemeter since sleep current is ~10uA. See Basic Communications - End Device for
 * information about measuring sleeping current consumption.
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

/** The current we will measure */
uint16_t current = 0;

/** Measures the current multiple times and averages the measurements to get a more accurate reading */
static uint16_t getAverageCurrent()
{
#define NUMBER_OF_SAMPLES           16    
#define SAMPLE_DELAY_MS             50
    uint32_t total = 0;
    uint8_t i = 0;
    for (i = 0; i< NUMBER_OF_SAMPLES; i++)
    {
        total += getCurrentSensor();
        delayMs(SAMPLE_DELAY_MS);
    }
    return ( (uint16_t) (total/ NUMBER_OF_SAMPLES) );
}

int main( void )
{
    halInit();
    moduleInit();    
    printf("\r\n************************************************\r\nMeasure Module Current\r\n");
    printf("\r\nResetting Radio, then measuring current\r\n");    
    if (moduleReset() == MODULE_SUCCESS)
    {
        displaySysResetInd();  // Display the contents of the received SYS_RESET_IND message
    } else {
        printf("ERROR\r\n");
    }
    printf("Starting current measurement\r\n");

    while (1)
    {
        /* Turn ON the RF Test output. At -10dBm RF output, Module consumes approximately 70mA */
        if (setRfTestMode(RF_TEST_UNMODULATED, 15, RF_OUTPUT_POWER_MINUS_10_0_DBM, 0) != MODULE_SUCCESS)
            printf("ERROR in setRfTestMode()\r\n");
        /* RESET the Module to apply these RF Test settings */
        setLed(2);
        RADIO_OFF();
        delayMs(1);
        RADIO_ON();

        /* Measure current consumed by module with RF test output ON */
        printf("Module RF Test Output ON\r\n");
        delayMs(1000);
        current = getAverageCurrent();
        printf("Current = %u.%umA\r\n", current/10, current%10);
        delayMs(1000);

        /* Now, resetting the module will stop the RF test output and the module will be in an IDLE state
       It is NOT asleep, just hanging out and waiting to be configured.
       Module in idle state consumes approximately 26mA */
        clearLed(2);
        RADIO_OFF();
        delayMs(1);
        RADIO_ON();

        /* Measure current consumed by module with RF test output OFF */
        printf("Module RF Test Output OFF\r\n");
        delayMs(1000);
        current = getAverageCurrent();
        printf("Current = %u.%umA\r\n", current/10, current%10);
        delayMs(1000);
    }
}

/* @} */
