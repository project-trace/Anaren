/**
 * @ingroup hwExamples
 * @{
*
* @file example_button_interrupt.c
*
* @brief Toggles LED0 when button pressed.
* 
* Configures pin controlling LED0 and LED1 to be an output. Configures pin attached to button as an input with interrupt. 
* Once per second toggles LED0 as a simple indication that application is running.
* When interrupt generated, waits to allow button to debounce then if button pressed, toggles LED0.
*
* This example is a good way to verify that the development environment is configured correctly.
*
* This example makes use of function pointers in the HAL file. 
* These get called by the appropriate ISR in the HAL file when the appropriate interrupt is fired. 
* To use a function pointer:
* - Declare it in this file:              extern void (*buttonIsr)(void);
* - Assign it to your local function      buttonIsr = &handleButtonPress;
* Now, whenever the button is pressed, your local handleButtonPress() method will be called.
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

/** function pointer (in hal file) for the function that gets called when a button is pressed*/
extern void (*buttonIsr)(int8_t);

/** Our button interrupt handler */
void handleButtonPress(int8_t button);

int main( void )
{
    halInit();                          //Initialize hardware
    buttonIsr = &handleButtonPress;
    HAL_ENABLE_INTERRUPTS();              //Enable Interrupts
    while (1) 
    {
        toggleLed(1);         
        delayMs(1000);          
    }
}

/** Button interrupt service routine. Called when interrupt generated on the button.
 @param button which button was pressed.
@pre Button connects input to GND.
@pre pins are configured as interrupts appropriately and have pull-UP resistors.
*/
void handleButtonPress(int8_t button)
{
    toggleLed(0);
}

/* @} */
