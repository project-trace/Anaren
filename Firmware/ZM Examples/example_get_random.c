/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_get_random.c
 *
 * @brief Resets Radio, gets a random number using SYS_RANDOM command and displays it.
 *
 * Configures the microcontroller to communicate with the radio, resets the radio, and sends the
 * SYS_RANDOM command and parses the received response and displays it to the console unless there
 * was an error. Demonstrates basic request-response functionality of the module.
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
#include "../Common/utilities.h"
#include <stdint.h>

moduleResult_t result = MODULE_SUCCESS;

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

int main( void )
{
    halInit();
    moduleInit();  
    printf("\r\nResetting Radio, then getting Random Number\r\n");
    moduleReset();   
    while (1)
    {
        /* Get a random number from the module */
        result = sysRandom();
        if (result == MODULE_SUCCESS)                  
        {
            /* Random number is in zmBuf. Now we use a convenience macro to read result from zmBuf */
            uint16_t randomNumber = SYS_RANDOM_RESULT();    
            printf("Random Number = %u (0x%04X)\r\n", randomNumber, randomNumber);
        } else {
            printf("ERROR 0x%02X\r\n", result);
        }
        delayMs(1000);
    }
}

/* @} */


