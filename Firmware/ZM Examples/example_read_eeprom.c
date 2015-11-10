/**
 * @ingroup hwExamples
 * @{
 * @file example_read_eeprom.c
 *
 * @brief reads and writes the EEPROM
 *
 * Uses the Hardware Abstraction Layer (hal).
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
#include "../HAL/hal_Microchip_24xxxxx_eeprom.h"
#include "../Common/utilities.h"
#include <stdint.h>
#include <string.h>

/** Simple buffer equal to page size of EEPROM, used for page reads and writes */
uint8_t pageBuffer[EEPROM_PAGE_SIZE];  

/** Which EEPROM byte address to use for single byte testing */
#define TEST_ADDRESS    3

/** Which EEPROM byte address to use for whole page testing.
Must be the start of a page */
#define TEST_PAGE_ADDRESS   0

/* What value to write to the EEPROM for single byte testing.
Note: don't use 0xFF since that is the value of a blank EEPROM. */
#define TEST_VALUE      0x77    

uint8_t writeCounter = 0;

int main( void )
{
    halInit();
    printf("Read/Write EEPROM\r\n");
    eepromInit();

    while (1) 
    {       
        // Page wide Transactions
        printf("\r\nWHOLE PAGE READ/WRITES\r\n");

        // Initialize our Write Buffer with DE AD BE EF
        printf("Initializing Write Buffer:\r\n");        
        initializeBuffer(pageBuffer, EEPROM_PAGE_SIZE);        

        // Page Write to EEPROM
        printf("Writing DEADBEEF pattern to EEPROM page %u\r\n", TEST_PAGE_ADDRESS);
        eepromPageWrite(TEST_PAGE_ADDRESS, pageBuffer, EEPROM_PAGE_SIZE);
        writeCounter = 0;
        // Now, wait until the EEPROM is not busy writing before doing anything else
        // This is done with "Acknowledge Polling" - the EEPROM won't ack until it's done writing
        while (eepromBusy())
        {
            writeCounter++;
            delayMs(1);
        }
        printf("EEPROM finished write in %umS\r\n", writeCounter);

        // Initialize Buffer to 0 for reading
        printf("Initializing Read Buffer to all 0\r\n");
        memset(pageBuffer, 0, EEPROM_PAGE_SIZE);

        // Read from EEPROM
        printf("Reading EEPROM page %u\r\n", TEST_PAGE_ADDRESS);
        eepromSequentialRead(TEST_PAGE_ADDRESS, pageBuffer, EEPROM_PAGE_SIZE);
        // Display Bytes Read
        printf("Displaying Bytes Read:\r\n");
        printHexBytes(pageBuffer, EEPROM_PAGE_SIZE);


        // Single Byte Transactions
        printf("\r\nSINGLE BYTE READ/WRITES\r\n");

        // Test Random Write
        printf("Writing %02X to %04X\r\n", TEST_VALUE, TEST_ADDRESS);
        eepromByteWrite(TEST_ADDRESS, TEST_VALUE);
        writeCounter = 0;
        // Again, wait until the EEPROM is not busy writing before doing anything else
        while (eepromBusy())
        {
            writeCounter++;
            delayMs(1);
        }
        printf("EEPROM finished write in %umS\r\n", writeCounter);

        // Test Random Read of the same byte
        printf("Read %02X from same location\r\n", eepromRandomRead(TEST_ADDRESS));

        // Test Read Current Address
        printf("Now demonstrate Read Current Address: ");
        printf(" %02X ", eepromReadCurrentAddress());
        printf(" %02X ", eepromReadCurrentAddress());
        printf(" %02X ", eepromReadCurrentAddress());                
        printf(" %02X ", eepromReadCurrentAddress());

        printf("\r\n\r\n");


        delayMs(2000);
    }

}

/* @} */
