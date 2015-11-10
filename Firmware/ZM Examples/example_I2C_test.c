/**
* @ingroup hwExamples
* @{ 
* @file example_I2C_test.c
*
* @brief tests the I2C interface
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
#include "../HAL/hal_TI_TMP006_IR_temperature_sensor.h"
#include "../HAL/hal_AMS_TCS3414_color_sensor.h"
#include "../HAL/hal_Microchip_24xxxxx_eeprom.h"
#include "../Common/utilities.h"
#include <stdint.h>
#include <string.h>

#include "module_example_utils.h"

int main( void )
{
    halInit();
    printf("I2C Test\r\n");

    i2cInit(0x50);

#ifdef ADDRESS_SEARCH       // This searches the I2C bus and displays all found slaves
    i2cInit(0x39);          // Can use any valid I2C address; doesn't matter
    i2cAddressSearch();     // Display a list of which I2C devices are attached
    printf("\r\n");
    delayMs(100);
#endif
    
#ifdef TEST_TMP006
    tmp006Init();
#endif
    
#ifdef TEST_TCS3414
    colorSensorInit();
#endif    
    
#ifdef TEST_EEPROM
    eepromInit();
#endif 

    while (1) 
    {

delayMs(100);
#ifdef TEST_TMP006
        printf("Getting IR Temperature:\r\n");
        struct TempReading currTemp;        // Calculated temperature value returned by getTemp
        tmp006GetTemperature(&currTemp);    // Get current temperature from TMP006. Returns raw values

        long double tempDie = (currTemp.tDie >> 2) * 0.03125;
        int16_t tempDieInt = (int16_t) (tempDie * 100.0);
        printf("Die Temperature = %d.%03dC\r\n", tempDieInt/100, tempDieInt%100);
     
        long double tempF = ( ((currTemp.temp * (1.8)) + 32.0 ) );
        int16_t tempFint = (int16_t) (tempF * 100.0);
        
        printf("IR Temperature = %d.%02dC (%d.%02dF)\r\n\r\n", currTemp.tempInt/100, (currTemp.tempInt%100), tempFint/100, tempFint%100);
#endif
delayMs(100);
#ifdef TEST_TCS3414
        struct ColorReading color;
        printf("Getting Color Reading:\r\n");
        color = getColor();
        printf("RED: %05u - BLUE: %05u - GREEN: %05u - CLEAR: %05u\r\n\r\n", color.red, color.blue, color.green, color.clear);        
#endif

#ifdef TEST_EEPROM
        eepromInit();
        uint16_t test = 0;
        uint16_t writeCounter = 0;
#define TEST_ADDRESS    EEPROM_PAGE_SIZE // use next page for single byte read/write tests
#define TEST_VALUE      0x77    // Note: don't use 0xFF since that is the value of a blank EEPROM.
        test = eepromRandomRead(TEST_ADDRESS);
        printf("Initial Value @%02X=0x%02X - ", TEST_ADDRESS, test);
        eepromByteWrite(TEST_ADDRESS, TEST_VALUE);
        printf("Write @%02X=0x%02X - ", TEST_ADDRESS, TEST_VALUE);
        // must wait for read to finish:
        while (eepromBusy())
        {
            writeCounter++;
            delayMs(1);
        }
        printf("Finished write in %umS - ", writeCounter);
        test = eepromRandomRead(TEST_ADDRESS);
        printf("Read @%02X=0x%02X\r\n", TEST_ADDRESS, test);

        /* Initialize Write Buffer with DEADBEEF */
        printf("Initializing Write Buffer - ");
        uint8_t tempBuffer[EEPROM_PAGE_SIZE];
        initializeBuffer(tempBuffer, EEPROM_PAGE_SIZE);        

        /* Write to EEPROM */
        printf("Testing eepromPageWrite - ");
        eepromPageWrite(0, tempBuffer, EEPROM_PAGE_SIZE);
        writeCounter = 0;
        while (eepromBusy())
        {
            writeCounter++;
            delayMs(1);
        }
        printf("Finished write in %umS\r\n", writeCounter);

        /* Initialize Read Buffer to 0 */
        printf("Initializing Read Buffer - ");
        uint8_t temp[EEPROM_PAGE_SIZE] = { 0 };

        /* Read from EEPROM */        
        printf("Testing Sequential Read - ");
        eepromSequentialRead(0, temp, EEPROM_PAGE_SIZE);
        /* Display Bytes Read */
        printf("Displaying Bytes Read:\r\n");
        printHexBytes(temp, EEPROM_PAGE_SIZE);

        /* Test Read Current Address */
        printf("Now demonstrate eepromReadCurrentAddress: ");
        eepromRandomRead(0);
        printf(" %02X ", eepromReadCurrentAddress());
        printf(" %02X ", eepromReadCurrentAddress());
        printf(" %02X ", eepromReadCurrentAddress());                
        
        printf("\r\n");
        
#endif        
        
#ifdef ADDRESS_SEARCH       // This searches the I2C bus and displays all found slaves
    i2cInit(0x39);          // Can use any valid I2C address; doesn't matter
    i2cAddressSearch();     // Display a list of which I2C devices are attached
    printf("\r\n");
#endif

    printf("\r\n");
        delayMs(1000);
    }
}

/* @} */
