/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_get_mac_address.c
 *
 * @brief Resets Module, gets MAC Address
 *
 * Configures the processor to communicate with the module, resets the module, and uses the
 * ZB_GET_DEVICE_INFO command to get the MAC address.
 *
 * ZB_DEVICE_INFO can be used to obtain valuable information about the operation of Module. Some
 * fields like DIP_MAC_ADDRESS are valid all the time but the Zigbee network information like
 * DIP_SHORT_ADDRESS is only valid once the Module has joined a network successfully. Note that all
 * values returned from the module are Least Significant Byte (LSB) first. This example demonstrates
 * how to flip the byte order so that the correct value is displayed.
 *
 * These device information properties are useful for debugging. When troubleshooting a connection
 * problem, read DIP_STATE as it will give you an indication of what the Module is doing and whether
 * it is on a network. Also it is handly to display all DIPs after the Module has joined the network.
 *
 * @note throughout the Module, data is always sent/received  with least significant byte first.
 * For example, if sending data 0x01020304 then you would send byte 0x04 first and 0x01 last. This is
 * why we swap the byte order of the MAC address in this example.
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
#include "../ZM/zm_phy.h"
#include <stdint.h>

moduleResult_t result = MODULE_SUCCESS;

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

int main( void )
{
    halInit();
    moduleInit();    
    printf("\r\nResetting Module, then getting MAC Address\r\n");
    HAL_ENABLE_INTERRUPTS();

    result = moduleReset();
    if (result == MODULE_SUCCESS)
    {
        /* Display the contents of the received SYS_RESET_IND message */
        displaySysResetInd();  
    } else {
        printf("ERROR 0x%02X\r\n", result);
    }    

    while (1)
    {
        result = zbGetDeviceInfo(DIP_MAC_ADDRESS);
        if (result == MODULE_SUCCESS)
        {
            uint8_t* mac = zmBuf+SRSP_DIP_VALUE_FIELD;
            printf("MAC (as sent, LSB first):");
            printHexBytes(mac, 8);

            /* Note: the MAC address comes over the wire in reverse order (LSB first)
            So we swap the order of the bytes so we can display it correctly. */
            uint8_t temp[8];
            int i;
            for (i=0; i<8; i++)
            {
                temp[i] = mac[7-i];
            }
            printf("MAC (correct, MSB first):");
            printHexBytes(temp, 8);
            printf("\r\n");
        }
        else
            printf("ERROR 0x%02X\r\n", result);
        delayMs(1000);
    }
}

/* @} */
