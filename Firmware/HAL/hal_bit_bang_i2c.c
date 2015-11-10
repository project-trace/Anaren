/**
* @ingroup hal
* @{ 
* @file hal_bit_bang_i2c.c
*
* @brief Software I2C interface. Contains individual methods (start/stop/in/out) as well as full methods (read, write) etc.
*
* Uses the Hardware Abstraction Layer (hal).
*
* I2C Notes:
* - I2C direction bit is the last bit of the address byte, right before ACK bit. ZERO indicates a 
* write to the slave and ONE indicates a read from the slave. After the direction bit and 
* ACKNOWLEDGE, master will send the data to the slave or read data from the slave depending on the 
* direction. The data transfer will be terminated when master sets STOP condition on the bus.
* - According to the I2C standard, when communicating on bus the master never outputs a logic high. 
* It only pulls the bus down. Logic high is set by allowing external pull-ups to pull the bus up.
*
* $Rev: 1528 $
* $Author: dsmith $
* $Date: 2012-08-30 15:58:08 -0700 (Thu, 30 Aug 2012) $
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

#include "hal_launchpad.h"
#include "../Common/printf.h"
#include "hal_bit_bang_i2c.h"

/* Various delays used in the bit-bang routines. May need adjusting depending on your hardware. */
#define BIT_TIME_5_US   4
#define BIT_TIME_2_US   2
#define DELAY_FULL()     (__delay_cycles(TICKS_PER_US*BIT_TIME_5_US))
#define DELAY_PART()     (__delay_cycles(TICKS_PER_US*BIT_TIME_2_US))
#define DELAY_HALF()     (__delay_cycles(TICKS_PER_US))

/* I2C Address configuration */
#define I2C_ADDRESS_NOT_DEFINED 0xFF
#define I2C_INTERFACE_NOT_INTIALIZED()    (address == I2C_ADDRESS_NOT_DEFINED)
#define IS_VALID_I2C_ADDRESS(addr)  ((addr > 0x07) && (addr < 0x78))

/** Stores the address of the slave device we wish to talk to. Must be configured before use. */
uint8_t address = I2C_ADDRESS_NOT_DEFINED;

/* Bit-bang operations - Output */
#define SET_SDA_HIGH()   (I2C_SDA_DIR_PORT &= ~I2C_MASTER_SDA)  // set to high by tri-stating bus (external pull-up)
#define SET_SCL_HIGH()   (I2C_SCL_DIR_PORT &= ~I2C_MASTER_SCL)  // set to high by tri-stating bus (external pull-up)
#define SET_SDA_LOW()   (I2C_SDA_DIR_PORT |= I2C_MASTER_SDA)
#define SET_SCL_LOW()   (I2C_SCL_DIR_PORT |= I2C_MASTER_SCL)

/** Read the state of the SDA line */
#define READ_SDA()  ( I2C_SDA_IN_PORT & I2C_MASTER_SDA )     

/* Set outputs to 0 - only used in initialization */
#define CONFIGURE_SDA_OUTPUT_LOW()   (I2C_SDA_OUT_PORT &= ~I2C_MASTER_SDA)  // set to low
#define CONFIGURE_SCL_OUTPUT_LOW()   (I2C_SCL_OUT_PORT &= ~I2C_MASTER_SCL)  // set to low


//
//  Individual Methods - these are used in wrapper methods, below
//

/** Initialization code. This must be called before any other method. Stores the I2C address we wish
to talk to.
@pre SDA and SCL were configured as GPIOs.
@param address which I2C address we will be communicating with. Must be a valid I2C address, between
0x08 and 0x77, inclusive. For example, a TI TMP006 has an address of 0x40; the TAOS TCS3414 has an 
address of 0x39.
@note some manufacturers give a "read address" and "write address". To get the true I2C address, 
discard the LSB and shift this one bit to the right.
@return 0 if success, else -1 if invalid i2c Address.
*/
int8_t i2cInit(uint8_t i2cAddress)
{
    if (!(IS_VALID_I2C_ADDRESS(i2cAddress)))
    {
#ifdef VERBOSE_I2C
        printf("Invalid I2C Address\r\n");
#endif
        return -1;
    } else {
        address = i2cAddress;
        return 0;
    }
}

/**
Displays the i2c error(s) based on the contents of bitfield i2cErr.
 */
void displayI2cError(unsigned long i2cErr)
{
	printf("I2C Errors (%02X)\r\n", i2cErr);
}

/** Sends an I2C Start condition. A HIGH to LOW transition on the SDA line while SCL is HIGH 
indicates a START condition.
@note Only a master can establish START condition on the bus. The slaves should detect the START 
condition and be ready to receive the ADDRESS and DATA. The bus is considered to be busy from the 
point a master establishes START. The bus is released by using STOP condition. 
@see http://www.i2cbus.com/theprotocol/start.html
@post both SDA & SCL are low
*/
void i2cStart( void )
{
	//Set both pins to outputs if not already by tri-stating bus (external pull-up will pull them high)
    SET_SDA_HIGH();
    SET_SCL_HIGH();
    
	//Set output to 0 here so that whenever we make SDA or SCL an output, it will be low
    CONFIGURE_SDA_OUTPUT_LOW();
    CONFIGURE_SCL_OUTPUT_LOW();

    DELAY_FULL();
	SET_SDA_LOW();	//Set SDALow, pause in case both pins were not high

    DELAY_FULL();
    SET_SCL_LOW();
}

/** Transmits one byte via I2C
@pre start has been sent, or this is not the first byte
@pre both SCL & SDA are low
@return true if ack was received
*/
uint8_t i2cOut(uint8_t data )
{
    uint8_t i = 0;
	for(i=0; i < 8; ++i )				// Iterate through all 8 bits, from MSB to LSB
	{
		if( data & 0x80 )				// output most significant bit
			SET_SDA_HIGH();
		else
			SET_SDA_LOW();
		
		data <<= 1;		//shift next data bit
        
        SET_SCL_HIGH();		// Toggle SCL, first high
        DELAY_FULL();
        SET_SCL_LOW();      // Then low
        DELAY_HALF();        
	}

	SET_SDA_HIGH();  // Set SDA as an input to be able to read the ACK.
	SET_SCL_HIGH();	//Set  SCL High
    
    // Note: we reuse i to save stack space
	i = !( READ_SDA() ); // Read SDA pin here. ACK is pulling the line low
	
	if( i ) //leave SDA in last state!
		SET_SDA_LOW();	
    DELAY_HALF();  
    
	SET_SCL_LOW();	//Set SCL Low
    
	return i;
}

/** Sends an I2C Start condition and sends the address of the device we want to talk to, for writing.
For writing, the LSB is set to 0. 
@pre i2c address was configured. This method does not check.
@see i2cStart()
*/
void i2cStartTx()
{
    i2cStart();
    i2cOut(address << 1);
}

/** Sends an I2C Start condition and sends the address of the device we want to talk to, for reading.
For reading, the LSB is set to 1. 
@pre i2c address was configured. This method does not check.
@see i2cStart()
*/
void i2cStartRx()
{
#define READ_BIT    0x01
    i2cStart();
    i2cOut((address << 1) | READ_BIT);
}


/** Reads one byte via I2C
@pre start has been sent, or this is not the first byte
@return the value read from the I2C bus
*/
uint8_t i2cIn()
{
    uint8_t data = 0;
    
    SET_SDA_HIGH();           // Release SDA
    
    uint8_t i= 0;
    for( ; i < 8; ++i )
    {
        SET_SCL_HIGH();       // Set Clock High
        
        data = data << 1;                        // Shift the bit over
        DELAY_HALF();
        
        if( READ_SDA() )    // Read SDA
            data|= 0x01;
        
        DELAY_HALF();
        SET_SCL_LOW();        //Set Clock Low
        DELAY_PART();
    }
    return data;
}

/** Sends an I2C Stop condition. A LOW to HIGH transition on the SDA line while SCL is HIGH 
indicates a STOP condition.
@note Master devices establish STOP condition on the bus to let the slaves know that the message is 
finished. The bus is considered to be busy until master sets STOP condition. After the STOP, the bus 
is free and any other masters can claim the bus for communication adhering to the arbitration process.
@pre SDA is low
*/
void i2cStop( void )
{
    SET_SDA_LOW();    //make sure SDA is low
    DELAY_FULL();    //SCL to high, pause in case SDA was high
    SET_SCL_HIGH();
    DELAY_FULL();
    SET_SDA_HIGH();    //SDA to high
}


//
//  Wrapper methods - these use the above methods for more advanced operations
//

/** Reads the specified number of bytes from the I2C port.
@pre bytes is large enough to hold count number of bytes.
@note Assumes the IC2_MASTER_SCL is low
@note Order: Start, ADDRESS, first byte ... last byte, NAK,stop
@param bytes where the received bytes will be written to
@param numBytes the number of bytes to be read
@post bytes contains the bytes read from the I2C interface
*/
uint8_t i2cRead(uint8_t* bytes, uint8_t numBytes)
{
    i2cStartRx();
    
    while (numBytes--)
    {
        *bytes++= i2cIn();        //put the input data byte into the buffer, increment buffer pointer
        
        if( numBytes )             //No Ack after last byte
            SET_SDA_LOW();
        
        SET_SCL_HIGH();        //Toggle SCL, first high
        DELAY_FULL();
        SET_SCL_LOW();        //Set SCL Low
    }
    i2cStop();
    
    return 0;
}

/** Special method for generating a NAK after only one byte is read. Use with caution.
Order: Start, ADDRESS, byte, NAK, stop.
@note this can be used in place of legacy method i2cReadCurrentAddress().
@param value where to read the byte into
@return 0 if success, else an error code
*/
uint8_t i2cReadOneByte(uint8_t* value)
{
    return (i2cRead(value, 1));
}

/** Writes to the specified address, looks to see if it was ACK'd.
@return 1 if that address responded to an ACK, else 0
@param addressToTest the address to check
*/
uint8_t i2cAddressTest(uint8_t addressToTest)
{
    i2cStart();
    uint8_t ack = i2cOut(addressToTest << 1);
    i2cStop();
    return ack;
}

/** Searches the entire i2c address space (0x00 through 0x7F) for devices which return an ACK. 
Displays to console which addresses have ACK'd.
@pre I2C bus has been initialized using i2cInit(). Address used does not matter.
*/
void i2cAddressSearch(void)
{
    if (I2C_INTERFACE_NOT_INTIALIZED())
    {
        printf("Error - bus not initialized\r\n");
        return;
    }
    printf("Searching the I2C bus - Addresses Found (in hex): ");
    int numberOfDevicesFound = 0;
    for (uint8_t addressToTest = 0; addressToTest < 0x7F; addressToTest++)
    {
        if (i2cAddressTest(addressToTest) == 1)
        {
            printf("%02X ", addressToTest);
            numberOfDevicesFound++;   
        }
    }
    printf("\r\nTest Done, %u devices found\r\n", numberOfDevicesFound);
}

/** Writes the specified number of bytes out the I2C port.
@note Order: Start, ADDRESS, first byte ... last byte, stop
@pre bytes contains the bytes to be written
@param bytes where the received bytes will be written to
@param numBytes the number of bytes to be read
*/
uint8_t i2cWrite(uint8_t* bytes, uint8_t numBytes)
{
#ifdef VERBOSE_I2C    
    printf("writing to i2c %u bytes:\r\n", numBytes);
    for (int i=0; i<numBytes; i++)
        printf("%02X ", bytes[i]);
    printf("\r\n");
#endif
    
    i2cStartTx(); 
    
#ifdef ERROR_HANDLING
    uint8_t error = 0xFF;  
    for (int i=0; i<numBytes; i++)
    {
        if (!(i2cOut(bytes[i])))
        {
            error = i;   // if NACK
            break;
        }
    }
    i2cStop();  //note: generate a STOP condition even if there was an error
    if (error != 0xFF)
        printf("error on byte %02X\r\n", error);
    return error;
#else
    for (int i=0; i<numBytes; i++)
    {
        i2cOut(bytes[i]);
    }
    i2cStop();
    
    return 0;
#endif
}

/* @} */
