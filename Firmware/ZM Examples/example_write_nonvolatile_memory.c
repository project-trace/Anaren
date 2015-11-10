/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_write_nonvolatile_memory.c
 *
 * @brief Resets Module, prompts the user for which NV item to write (1-6) and writes test data to that location.
 *
 * The Module supports six NV storage buffers (named "NV Items"). 1-4 are 2 bytes each; 5 & 6 are 16 bytes each.
 * These NV items are stored in the Module's flash memory and can be used for non-volatile storage.
 *
 * @see example_read_nonvolatile_memory.c
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
#include "../Common/utilities.h"
#include "../ZM/module_errors.h"
#include "../ZM/zm_phy_spi.h"
#include <stdint.h>

moduleResult_t result = MODULE_SUCCESS;

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

#define NO_CHARACTER_RECEIVED 0xFF

uint8_t getWhichNvItemToWrite();

/** The character the user entered. Must be volatile or else may be optimized out */
volatile char command;

uint8_t dataToWrite[16] = "*TESLA CONTROLS*";

/** function pointer (in hal file) for the function that gets called when a byte is received on the debug console. */
extern void (*debugConsoleIsr)(int8_t); 

void handleDebugConsoleInterrupt(int8_t rxByte);

int main( void )
{
    halInit();
    moduleInit();
    printf("\r\nWriting NV Items\r\n");
    result = moduleReset();
    if (result == MODULE_SUCCESS)
    {
        displaySysResetInd();  // Display the contents of the received SYS_RESET_IND message
    } else {
        printf("Module Reset ERROR 0x%02X\r\n", result);
    }
    debugConsoleIsr = &handleDebugConsoleInterrupt;   //call method handleDebugConsoleInterrupt() when a byte is received    
    HAL_ENABLE_INTERRUPTS();              //Enable Interrupts

    while (1)
    {
        uint8_t whichNvItem = getWhichNvItemToWrite();  
        if (whichNvItem != NO_CHARACTER_RECEIVED)
        {
            uint8_t nvItemSize = getNvItemSize(whichNvItem);
            printf("\r\nWriting to NV item %u, L%u:", whichNvItem, nvItemSize);    
            printHexBytes(dataToWrite, nvItemSize);
            result = sysNvWrite(whichNvItem, dataToWrite);
            if (result != MODULE_SUCCESS)
            {
                printf("sysNvWrite ERROR 0x%02X\r\n", result);
            }
        }
    }
}


/** 
Prompt the user to enter the number of an NV item. Blocking wait until user enters a valid number.
@return which NV item the user selected (1-6), or NO_CHARACTER_RECEIVED if none. 
 */
uint8_t getWhichNvItemToWrite()
{
    command = NO_CHARACTER_RECEIVED;     
    printf("Which NV Item to write to? (1-6)\r\n");
    while (command == NO_CHARACTER_RECEIVED) ;  //wait until user types a character
    if ((command < '1') || (command > '6')) 
    {
        printf("\r\nMust be from 1 to 6, inclusive.\r\n"); 
        return NO_CHARACTER_RECEIVED; 
    }

    return (command - '0');
}


/** 
 Method to handle bytes received on the debug console.
This gets called by the ISR in the hal file since we set the debugConsoleIsr function pointer
(in hal file) to point to this function.
 */
void handleDebugConsoleInterrupt(int8_t rxByte)
{
    command = rxByte;
}

/* @} */
