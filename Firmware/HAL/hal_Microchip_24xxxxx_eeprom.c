/**
* @ingroup hal
* @{ 
* @file hal_Microchip_24xxxxx_eeprom.c
*
* @brief Interface driver to Microchip EEPROM and equivalents (Atmel 24C etc.) via I2C.
*
*
* @see http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en010817
* @see http://www.atmel.com/devices/at24c128c.aspx
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

#include "hal.h"
#include "hal_Microchip_24xxxxx_eeprom.h"
#include "../Common/utilities.h"
#include <stdint.h>
#include <string.h>

/** Implements an i2c "random read" - reading the specified address from the EEPROM
@pre I2C bus was initialized with i2cInit(EEPROM_I2C_ADDRESS)
@param address which address to read
@return the value of the register, or -1 if error
*/
int16_t eepromRandomRead(uint16_t address)
{
    if (address > EEPROM_MAX_ADDRESS)
    {
        printf("Bad Address\r\n");
        return -1;
    }    
    uint8_t I2CBuffer[2];     // MSB first
    I2CBuffer[0] = MSB(address);
    I2CBuffer[1] = LSB(address);
    i2cWrite(I2CBuffer, 2);
    uint8_t result = i2cReadOneByte(I2CBuffer);
    if (result != 0)
    {
        printf("EEPROM Random Read ");
        displayI2cError(result);
    }
#ifdef EEPROM_VERBOSE
    printf("Read %02X from %04X\r\n", I2CBuffer[0], address);
#endif    
    return(I2CBuffer[0]);
}

/** Writes the value of a single 8 bit value to the EEPROM
@pre I2C bus was initialized with i2cInit(EEPROM_I2C_ADDRESS)
@param address which register to write to
@param value the value to be written to the EEPROM
@return 0 if success, else an error code.
*/
int16_t eepromByteWrite(uint16_t address, uint8_t value)
{
    if (address > EEPROM_MAX_ADDRESS)
    {
        printf("Bad Address\r\n");
        return -1;
    }
#ifdef EEPROM_VERBOSE
    printf("Write %02X to %04X\r\n", value, address);
#endif
    uint8_t I2CBuffer[3];     // MSB first
    I2CBuffer[0] = MSB(address);
    I2CBuffer[1] = LSB(address);
    I2CBuffer[2] = value;
    uint8_t result = i2cWrite(I2CBuffer, 3);
    if (result != 0)
    {
        printf("EEPROM Byte Write ");
        displayI2cError(result);
    }
    return 0;
}

/** Reads a number of bytes from the EEPROM
@pre I2C bus was initialized with i2cInit(EEPROM_I2C_ADDRESS)
@param address which address to start reading from
@param data where to write the data to
@param numBytes how many bytes to read.
@pre data is large enough to hold numBytes
@return 0 if success, else an error code.
*/
int16_t eepromSequentialRead(uint16_t address, uint8_t* data, uint8_t numBytes)
{
    if ((address > EEPROM_MAX_ADDRESS) || (numBytes == 0) || (numBytes > EEPROM_PAGE_SIZE) ||  (data == 0))
        return -1;
        
    uint8_t I2CBuffer[2];  

    I2CBuffer[0] = MSB(address);
    I2CBuffer[1] = LSB(address);     
    i2cWrite(I2CBuffer, 2);

    i2cRead(data, numBytes);

#ifdef EEPROM_VERBOSE
    printf("Read %uB starting at address %04X\r\n", numBytes, address);
#endif
    return 0;
}

/** Reads the address at the EEPROM's current address pointer. 
@return the value of the register
*/
uint8_t eepromReadCurrentAddress()
{
    uint8_t value = 0;
    uint8_t result = i2cReadOneByte(&value);
    if (result != 0)
    {
        printf("EEPROM Read Current Address ");
        displayI2cError(result);
    }
    return value;
}

/** Writes a number of bytes to the EEPROM
@pre I2C bus was initialized with i2cInit(EEPROM_I2C_ADDRESS)
@param address which address to start writing to
@param data the data to write
@param numBytes how many bytes to write.
@pre data holds the bytes to write
@return 0 if success, else an error code.
@note the EEPROM is not immediately available following this call since it takes ~5mSec to write.
See data sheet for write timing.
*/
int16_t eepromPageWrite(uint16_t address, uint8_t* data, uint8_t numBytes)
{
    if ((address > EEPROM_MAX_ADDRESS) || (numBytes == 0) || (numBytes > EEPROM_PAGE_SIZE) ||  (data == 0))
        return -1;
        
    uint8_t I2CBuffer[EEPROM_PAGE_SIZE + 2];  
    I2CBuffer[0] = MSB(address);
    I2CBuffer[1] = LSB(address);     
    memcpy(I2CBuffer + 2, data, numBytes);  // Copy data to write buffer
    
    i2cWrite(I2CBuffer, EEPROM_PAGE_SIZE + 2);
    
    return 0;
}

/** Initialize the EEPROM.
@pre I2C interface on microcontroller was configured correctly
@note Part maximum baud rate 400kHz for I2C.
*/
void eepromInit()   
{
    printf("EEPROM Initialize\r\n");
    i2cInit(EEPROM_I2C_ADDRESS);
    delayMs(1);
}

/** Tests whether EEPROM is busy using Acknowledgment Polling
@return 1 if busy, 0 if not
*/
uint8_t eepromBusy()
{
    return (!(i2cAddressTest(EEPROM_I2C_ADDRESS)));
}



/* @} */
