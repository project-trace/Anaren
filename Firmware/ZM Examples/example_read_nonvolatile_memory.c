/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_read_nonvolatile_memory.c
 *
 * @brief Resets Module, reads out the contents of all user non-volatile (NV) items and displays to console.
 *
 * The Module supports six NV storage buffers (named "NV Items"). 1 through 4 are 2 bytes each; 5 & 6 are 16 bytes each.
 * These NV items are stored in the Module's flash memory and can be used for non-volatile storage.
 * To verify, run the Write NV Items example, power cycle the Module, and then run this example.
 * You should see that the values are retained through loss of power.
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

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];


moduleResult_t result = MODULE_SUCCESS;

int main( void )
{
    halInit();
    moduleInit();  
    printf("Displaying NV Items:\r\n");
    moduleReset();  

    while (1)
    {
        printf("Reading NV Items from Module:\r\n");
        uint8_t itemNum = 0;
        for (itemNum = MIN_NV_ITEM; itemNum < (MAX_NV_ITEM_USER+1); itemNum++)   //iterate through all NV items
        {
            result = sysNvRead(itemNum);
            if (result == MODULE_SUCCESS)
            {
                uint8_t* buf = zmBuf + SYS_NV_READ_RESULT_START_FIELD;
                printf("NV Item %u:", itemNum);
                int i;
                for (i=0; i<getNvItemSize(itemNum); i++)
                    printf("%02X(%c)", buf[i], buf[i]);
                printf("\r\n");
            } else {
                printf("ERROR 0x%02X\r\n", result); 
            }
        }
        printf("\r\n");
        delayMs(3000);
    }
}

/* @} */
