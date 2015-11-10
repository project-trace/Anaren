/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_reset_module.c
 *
 * @brief Resets Module, parses message received and displays to console.
 *
 * Configures the microcontroller to communicate with the Module, asserts the Module's RESET line,
 * and then receives the SYS_RESET_IND message sent from the Module.
 *
 * If this isn't working correctly:
 * - Verify that CFG pins are set correctly.
 * - Verify that nothing else is connected to the RESET line, like an external programmer or debugger.
 * - Run the Hello World application to verify that the console is working.
 * - If all else fails, attach a logic analyzer and look at the traffic to/from the module. Compare with example traces.
 *
 * @note The Module can be reset through hardware (the RESET pin) or software (SYS_RESET_REQ).
 * It is highly recommended to use the hardware reset instead of the software reset.
 * @note This application is the foundation of all radio examples. If porting these examples to another processor,
 * get this example working before attempting any of the other radio examples.
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
#include <stdint.h>

moduleResult_t result = MODULE_SUCCESS;

int main( void )
{

    halInit();
    moduleInit();
    printf("\r\nResetting Module\r\n"); 
    HAL_ENABLE_INTERRUPTS();    
    moduleReset();
    while (1)
    {
        printf("Module Reset: ");
        result = moduleReset();
        if (result == MODULE_SUCCESS)
        {
            displaySysResetInd();  // Display the contents of the received SYS_RESET_IND message
        } else {
            printf("ERROR 0x%02X\r\n", result);
        }
        delayMs(3000);
    }
}


/* @} */
