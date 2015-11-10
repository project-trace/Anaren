/**
 * @ingroup hal
 * @{
 * @file hal_stellaris_softi2c.c
 *
 * @brief Software I2C interface for Stellaris processors. Originally based upon StellarisWare
 * example file StellarisWare/examples/peripherals/i2c/soft_i2c_atmel.c. Uses the software I2C
 * driver in StellarisWare. See StellarisWare/utils/softi2c.c for more information.
 *
 * I2C Notes:
 * - I2C direction bit is the last bit of the address byte, right before ACK bit. ZERO indicates a
 * write to the slave and ONE indicates a read from the slave. After the direction bit and
 * ACKNOWLEDGE, master will send the data to the slave or read data from the slave depending on the
 * direction. The data transfer will be terminated when master sets STOP condition on the bus.
 * - According to the I2C standard, when communicating on bus the master never outputs a logic high.
 * It only pulls the bus down. Logic high is set by allowing external pull-ups to pull the bus up.
 *
 * @note Zigbee BoosterPack pin assignments:
- SDA is on J2-1 on BoosterPack, which is PA2
- SCL is on J2-2 on BoosterPack, which is PA3
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

#include "hal_ek-lm4f120XL.h"
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "utils/softi2c.h"
#include "utils/uartstdio.h"
#include "hal_stellaris_softi2c.h"


/** I2C Slave Address */
static uint8_t SLAVE_ADDR = 0x10;

//*****************************************************************************
//
// The states in the interrupt handler state machine.
//
//*****************************************************************************
#define STATE_IDLE              0
#define STATE_WRITE_NEXT        1
#define STATE_WRITE_FINAL       2
#define STATE_WAIT_ACK          3
#define STATE_SEND_ACK          4
#define STATE_READ_ONE          5
#define STATE_READ_FIRST        6
#define STATE_READ_NEXT         7
#define STATE_READ_FINAL        8
#define STATE_READ_WAIT         9

#define STATE_WAIT_UNTIL_DONE   10

/** The state of the SoftI2C module. */
static tSoftI2C g_sI2C;

/** Pointer to the data to be transmitted or received. */
static unsigned char *g_pucData = 0;

/** The number of bytes to be transmitted or received. */
static unsigned long g_ulCount = 0;

/**The current state of the interrupt handler state machine. */
static volatile unsigned long g_ulState = STATE_IDLE;


/** The callback function for the SoftI2C module. */
void
SoftI2CCallback(void)
{
    // Clear the SoftI2C interrupt.
    SoftI2CIntClear(&g_sI2C);

    // Determine what to do based on the current state.
    switch(g_ulState)
    {
    // The idle state.
    case STATE_IDLE:
    {
        // There is nothing to be done.
        break;
    }

    // The state for the middle of a burst write.
    case STATE_WRITE_NEXT:
    {
        // Write the next data byte.
        SoftI2CDataPut(&g_sI2C, *g_pucData++);
        g_ulCount--;

        // Continue the burst write.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_CONT);

        // If there is one byte left, set the next state to the final write state.
        if(g_ulCount == 1)
        {
            g_ulState = STATE_WRITE_FINAL;
        }

        // This state is done.
        break;
    }

    // The state for the final write of a burst sequence.
    case STATE_WRITE_FINAL:
    {
        // Write the final data byte.
        SoftI2CDataPut(&g_sI2C, *g_pucData++);
        g_ulCount--;

        // Finish the burst write.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_FINISH);

        g_ulState = STATE_WAIT_UNTIL_DONE; //STATE_IDLE;

        // This state is done.
        break;
    }

    // The state for a single byte read.
    case STATE_READ_ONE:
    {
        //printf(" ~r1~ ");

        // Put the SoftI2C module into receive mode.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

        // Perform a single byte read.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_RECEIVE);

        // The next state is the wait for final read state.
        g_ulState = STATE_READ_WAIT;

        // This state is done.
        break;
    }

    // The state for the start of a burst read.
    case STATE_READ_FIRST:
    {
        //printf(" ~rF~ ");

        // Put the SoftI2C module into receive mode.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

        // Start the burst receive.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_START);

        // The next state is the middle of the burst read.
        g_ulState = STATE_READ_NEXT;

        // This state is done.
        break;
    }

    // The state for the middle of a burst read.
    case STATE_READ_NEXT:
    {
        //printf(" ~rN~ ");

        // Read the received character.
        *g_pucData++ = SoftI2CDataGet(&g_sI2C);
        g_ulCount--;

        // Continue the burst read.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_CONT);

        // If there are two characters left to be read, make the next
        // state be the end of burst read state.
        if(g_ulCount == 2)
        {
            g_ulState = STATE_READ_FINAL;
        }

        // This state is done.
        break;
    }

    // The state for the end of a burst read.
    case STATE_READ_FINAL:
    {
        //printf(" ~rX~ ");

        // Read the received character.
        *g_pucData++ = SoftI2CDataGet(&g_sI2C);
        g_ulCount--;

        // Finish the burst read.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_FINISH);

        // The next state is the wait for final read state.
        g_ulState = STATE_READ_WAIT;

        // This state is done.
        break;
    }

    // This state is for the final read of a single or burst read.
    case STATE_READ_WAIT:
    {
        // Read the received character.
        *g_pucData++ = SoftI2CDataGet(&g_sI2C);
        g_ulCount--;

        // The state machine is now idle.
        g_ulState = STATE_IDLE;

        // This state is done.
        break;
    }

    // This is a NEW state to wait until done
    case STATE_WAIT_UNTIL_DONE:
    {
        // Wait until the SoftI2C engine is done.
        while (SoftI2CBusy(&g_sI2C));

        // The state machine is now idle.
        g_ulState = STATE_IDLE;

        // This state is done.
        break;
    }
    default:
    {
        printf("State %u UNKNOWN\r\n", g_ulState);
        g_ulState = STATE_IDLE;
    }
    }
}


/**
Writes one or more bytes out via I2C, then reads the specified number of bytes in from the I2C port.
Order:  Start, ADDRESS(W), write first outputByte ... last outputByte, followed immediately by:
        Start, ADDRESS(R), read first byte ... last byte, NAK,stop
@param outputByte the bytes that shall be output
@param numOutputBytes the number of bytes to be output (typically 1 or 2)
@param bytes where the received bytes will be written to
@param numBytes the number of bytes to be read
@return 0 if success, else an error code
*/
uint8_t i2cBlockRead(uint8_t numOutputBytes, uint8_t* outputBytes, uint8_t numBytes, uint8_t* bytes)
{
#ifdef STELLARIS_SOFTI2C_VERBOSE
    printf("Write %u, Read %u\r\n", numOutputBytes, numBytes);
#endif

    if (numOutputBytes == 1)    // Single output byte
    {
        g_pucData = outputBytes;
        g_ulCount = numOutputBytes;

        g_ulState = STATE_WAIT_UNTIL_DONE;

        // Start with a dummy write to get the address set in the EEPROM.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR , false);

        // Write the address to be written as the first data byte.
        SoftI2CDataPut(&g_sI2C, *outputBytes);

        // Perform a single send, writing the address as the only byte.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_SEND);

    } else { // multiple bytes to write first

        g_pucData = outputBytes + 1;
        g_ulCount = numOutputBytes-1;

        // Set the next state of the callback state machine
        if(g_ulCount == 1)
        {
            g_ulState = STATE_WRITE_FINAL;
        } else {
            g_ulState = STATE_WRITE_NEXT;
        }

        // Set the slave address and setup for a transmit operation.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, false);

        // Write the address to be written as the first data byte.
        SoftI2CDataPut(&g_sI2C, *outputBytes);

        // Start the burst cycle, writing the address as the first byte.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_START);
    }

    // Wait until the SoftI2C callback state machine is idle.
    while(g_ulState != STATE_IDLE)
    {
    }

    //  Write done, now read

    //Initialize Read
    g_pucData = bytes;
    g_ulCount = numBytes;

    if(numBytes == 1)
    {
        // Put the SoftI2C module into receive mode.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

        // Perform a single byte read.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_RECEIVE);

        // The next state is the wait for final read state.
        g_ulState = STATE_READ_WAIT;
    } else {
        // Put the SoftI2C module into receive mode.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

        // Start the burst receive.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_START);

        // The next state is the middle of the burst read.
        if(g_ulCount == 2)
        {
            g_ulState = STATE_READ_FINAL;
        } else {
            g_ulState = STATE_READ_NEXT;
        }
    }

    while(g_ulState != STATE_IDLE)
    {
    }

    return 0;
}


/**
 * Interrupt handler for the Timer0A interrupt.
 * @note this MUST be put into the startup_ccs.c file
 */
void
Timer0AIntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Call the SoftI2C tick function.
    SoftI2CTimerTick(&g_sI2C);
}


/** Initialize the SoftI2C interface.
 * @param i2cAddress the slave address we want to talk to.
 */
int8_t i2cInit(uint8_t i2cAddress)
{
    if (SoftI2CBusy(&g_sI2C))
    {
            printf("Error - Busy!\r\n");
            while (1);
    }
    if (g_ulState != STATE_IDLE)
    {
        printf("Error - Not Idle!\r\n");
        while (1);
    }

    // First, in case the interface has already been configured, disable the timer interrupt
    IntDisable(INT_TIMER0A);
    // ... and also disable the timer while we configure it.
    TimerDisable(TIMER0_BASE, TIMER_A);

    // Initialize the SoftI2C module, including the assignment of GPIO pins.
    memset(&g_sI2C, 0, sizeof(g_sI2C));
    SoftI2CCallbackSet(&g_sI2C, SoftI2CCallback);
    SoftI2CSCLGPIOSet(&g_sI2C, GPIO_PORTA_BASE, GPIO_PIN_3);
    SoftI2CSDAGPIOSet(&g_sI2C, GPIO_PORTA_BASE, GPIO_PIN_2);
    SoftI2CInit(&g_sI2C);

    // Enable the SoftI2C interrupt.
    SoftI2CIntEnable(&g_sI2C);

    // Configure the timer to generate an interrupt at a rate of 40 KHz.  This will result in an
    // I2C rate of 10 KHz.
#define I2C_BAUD_RATE   25000
    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / (I2C_BAUD_RATE * 4));
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    // Enable the timer interrupt.
    IntEnable(INT_TIMER0A);

    SLAVE_ADDR = i2cAddress;

    printf("Initialized with I2C Address 0x%02X\r\n", SLAVE_ADDR);

    return (0);
}

/** Simple test routine to exercise the softI2c functions. Not normally used. */
void i2cTest()
{
    uint8_t pucData[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint8_t data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF};

    i2cBlockRead(2, data, 2, pucData);
    delayMs(20);

    i2cWrite(data, 2);
    printf("Double write done\r\n");

    i2cRead(pucData, 2);
    printf("Double Read done\r\n");

    i2cWrite(data, 1);
    printf("Single write done\r\n");

    i2cRead(pucData, 1);
    printf("Single Read done\r\n");

    delayMs(10);
}








/** Writes the specified number of bytes out the I2C port.
@note Order: Start, ADDRESS, first byte ... last byte, stop
@pre bytes contains the bytes to be written
@param bytes where the received bytes will be written to
@param numBytes the number of bytes to be read
*/
uint8_t i2cWrite(uint8_t* outputBytes, uint8_t numOutputBytes)
{
#ifdef STELLARIS_SOFTI2C_VERBOSE
    printf("Write %uB\r\n", numOutputBytes);
#endif

    if (numOutputBytes == 1)    // Single output byte
    {
        g_pucData = outputBytes;
        g_ulCount = numOutputBytes;

        g_ulState = STATE_WAIT_UNTIL_DONE;

        // Start with a dummy write to get the address set in the EEPROM.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR , false);

        // Write the address to be written as the first data byte.
        SoftI2CDataPut(&g_sI2C, *outputBytes);

        // Perform a single send, writing the address as the only byte.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_SEND);

    } else { // multiple bytes to write first

        g_pucData = outputBytes + 1;
        g_ulCount = numOutputBytes-1;

        // Set the next state of the callback state machine
        if(g_ulCount == 1)
        {
            g_ulState = STATE_WRITE_FINAL;
        } else {
            g_ulState = STATE_WRITE_NEXT;
        }

        // Set the slave address and setup for a transmit operation.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, false);

        // Write the address to be written as the first data byte.
        SoftI2CDataPut(&g_sI2C, *outputBytes);

        // Start the burst cycle, writing the address as the first byte.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_SEND_START);
    }

    // Wait until the SoftI2C callback state machine is idle.
    while(g_ulState != STATE_IDLE)
    {
    }

    return 0;
}


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
#ifdef STELLARIS_SOFTI2C_VERBOSE
    printf("r%uB\r\n", numBytes);
#endif

    g_pucData = bytes;
    g_ulCount = numBytes;

    if(numBytes == 1)
    {
        // Put the SoftI2C module into receive mode.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

        // Perform a single byte read.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_RECEIVE);

        // The next state is the wait for final read state.
        g_ulState = STATE_READ_WAIT;
    } else {
        // Put the SoftI2C module into receive mode.
        SoftI2CSlaveAddrSet(&g_sI2C, SLAVE_ADDR, true);

        // Start the burst receive.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_BURST_RECEIVE_START);

        // The next state is the middle of the burst read.
        if(g_ulCount == 2)
        {
            g_ulState = STATE_READ_FINAL;
        } else {
            g_ulState = STATE_READ_NEXT;
        }
    }

    while(g_ulState != STATE_IDLE)
    {
    }

    return 0;
}


/**
Displays the i2c error(s) based on the contents of bitfield i2cErr.
@param i2cErr the bitfield containing the error.
 */
void displayI2cError(unsigned long i2cErr)
{
    printf("I2C Errors (%u): ", i2cErr);
    if (i2cErr == SOFTI2C_ERR_NONE)
    {
        printf("SOFTI2C_ERR_NONE\r\n");
        return;
    }

    // Since this is a bitflag, could be more than one error. Have to display all...
    if (i2cErr & SOFTI2C_ERR_ADDR_ACK)
        printf("SOFTI2C_ERR_ADDR_ACK ");
    if (i2cErr & SOFTI2C_ERR_DATA_ACK)
        printf("SOFTI2C_ERR_DATA_ACK ");

    printf("\r\n");
}


/** Special method for generating a NAK after only one byte is read.
Order: Start, ADDRESS, byte, NAK, stop.
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
        g_ulCount = 1;

        g_ulState = STATE_WAIT_UNTIL_DONE;

        // Start with a dummy write to get the address set in the EEPROM.
        SoftI2CSlaveAddrSet(&g_sI2C, addressToTest , false);
#define TEST_DATA_BYTE  0
        // Write a zero.
        SoftI2CDataPut(&g_sI2C, TEST_DATA_BYTE);

        // Perform a single send, writing the address as the only byte.
        SoftI2CControl(&g_sI2C, SOFTI2C_CMD_SINGLE_SEND);

        // Wait until the SoftI2C callback state machine is idle.
        while(g_ulState != STATE_IDLE)
        {
        }

    unsigned long err = (uint8_t) SoftI2CErr(&g_sI2C);

    return (err == SOFTI2C_ERR_NONE);
}

/** Searches the entire i2c address space (0x00 through 0x7F) for devices which return an ACK. 
Displays to console which addresses have ACK'd.
@pre I2C bus has been initialized using i2cInit(). Address used does not matter.
 */
void i2cAddressSearch(void)
{
    printf("Searching the I2C bus - Addresses Found (in hex): ");
    int numberOfDevicesFound = 0;
    uint8_t addressToTest;
    for (addressToTest = 0; addressToTest < 0x7F; addressToTest++)
    {
        if (i2cAddressTest(addressToTest) == 1)
        {
            printf("%02X ", addressToTest);
            numberOfDevicesFound++;   
        }
        delayMs(10);
    }
    printf("\r\nTest Done, %u devices found\r\n", numberOfDevicesFound);
}



/* @} */
