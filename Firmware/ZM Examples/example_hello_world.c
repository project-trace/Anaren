/**
 * @ingroup hwExamples
 * @{
 *
 * @file example_hello_world.c
 *
 * @brief Outputs "Hello World" to console and toggles LED approximately once per second.
 *
 * Configures the debug console UART pins and LED pin accordingly; loops and outputs text.
 *
 * If you aren't seeing the message:
 * - Verify that hardware is installed correctly:
 *       Open the Device Manager: Go to Start : Run... type in devmgmt.msc and press Return.
 *       Then click on the + symbol next to ports. Note the COM port number next to the ez430 board.
 * - Verify that your terminal program is configured correctly:
 *       If using HyperTerminal, open the Device Manager: Go to Start : Run... type in hypertrm.exe and press Return.
 *       We recommend TeraTerm, as it is much better.
 * - Verify that the COM port number is correct and the baud rate and other configuration is correct.
 * - Verify that the application is running - verify that LED is blinking.
 * - Verify that the application is outputting bytes on the serial port:
 *       Use an oscilloscope to probe the microcontroller's Tx pin and verify that the UART is working.
 * - Using the debugger, stop the processor and see where it stops. It normally should halt in SysCtlDelay().
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
#include <stdint.h>

int main( void )
{
    halInit();
    uint8_t counter = 0;
    while (1) 
    {
        printf("Hello World %u\r\n", counter++);  
        toggleLed(0);
        delayMs(1000);          
    }
}

/* @} */
