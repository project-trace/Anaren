#include "hal.h"            //point this to your HAL file

/**
* @ingroup hal
* @{
* @file hal_helper.c
*
* @brief Utilities to assist changing hardware platforms
*
* We designed the ZM library and examples to be easy to port to different hardware platforms.
* All interface to the hardware is done through one Hardware Abstraction Library (HAL) file. 
* To change hardware platforms you need to create a HAL file for your hardware platform.
* 
* These utility methods will help you if you are changing hardware platforms by providing simple test
* methods for the HAL methods required by the ZM library. You will need to supply your own hal_xxxx.c 
* and hal_xxxx.h files, where xxxx is the name of your hardware platform. If you are changing hardware 
* it is assumed that you are very familiar with your destination hardware platform.
*
* There are several tests, #define the one you want to run. Run them in order.
*
* To change hardware platforms:
* - You will need access to an oscilloscope and/or logic analyzer. Good low cost ($150) logic analyzer: www.saleae.com
* - Copy and rename the hal_mdb2.h and .c files for your hardware; e.g. hal_exp5438.c and hal_exp5438.h.
* - Include your new hal_xxxx.c file in this project, and remove the existing hal_mdb2.c file
* - Using the schematic of your target board, update the MACROS REQUIRED FOR Module in the .h file and all the methods in the .c file.
* - Go through the tests below in order.
* - Run the basic examples: button_blink, hello_world, and reset_radio.
* - Be sure that the target device is configured in your compiler correctly.
* - Next, run the basic communications examples.
* - We recommend you get the examples working first, and then do any optimizations, rather than the other way around.
*
* @note the documentation displayed below will depend on which test is defined.
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

/* Uncomment only ONE of the tests below to run that test */
/* GENERAL TESTS */
//#define TEST_DELAY            //STEP 1 - get basic delay, LEDs working
//#define TEST_PUTCHAR          //STEP 2 - get putchar() working so that printf() will work
//#define TEST_PRINTF           //STEP 3 - test printf() with a simple hello world application
/* MODULE SPI TESTS */
//#define TEST_ZM_RESET         //STEP 4 - verify ZM is being reset
//#define TEST_SPI_WRITE        //STEP 5 - verify the SPI port is configured correctly (if using SPI)
//#define TEST_SYS_RESET_IND    //STEP 6 - Do something useful - reset the ZM and display the response
//#define TEST_SYS_GET_MAC      //STEP 7 - Do something even more useful - query the ZM for the MAC Address
/* MODULE UART TESTS */
//#define TEST_SERIAL_ZM_RESET
//#define TEST_SERIAL
/* OTHER TESTS */
//#define TEST_UART_INTERRUPT       //Optional - blink an LED when a byte is received on the debug UART
#define TEST_TRISTATE_ALL_PORTS     //This isn't a test, just sets all mcu pins to inputs

#ifdef TEST_DELAY
/** Use this to verify that you've configured halInit(), toggleLed() and delayMs() correctly. 
Monitor LED with oscilloscope or logic analyzer to verify that delay is equal to ~50mSec. */
int main( void )
{
    halInit();
    setLed(1);
    delayMs(1000);
    clearLeds(); 
    delayMs(1000);        
    while (1) 
    { 
        setLed(0);
        delayMs(100);
        clearLeds();
        delayMs(100);
    }
}
#endif

#ifdef TEST_PUTCHAR
/** Use this to verify that putchar() is working. The examples make liberal use of printf() which 
requires a working putchar() method. You should see a steady stream of 'U' output to your terminal.
This test is also useful if you are implementing a software UART and need to verify bit timing. 
@note 9,600kHz = 104.1666uSec*/
int main( void )
{
    halInit();
    while (1)
    {
        toggleLed(0);
        printf("U");
        delayMs(1000);
    }
}
#endif

#ifdef TEST_PRINTF
/** Use this to verify that printf() is working. 
You should see "Hello World N" output once per second, with N increasing each time.
@pre working putchar() method.
*/
int main( void )
{
    halInit();
    unsigned int num = 0;
    while (1)
    {
        toggleLed(0);      
        toggleLed(1);           
        printf("Hello World %u\r\n", num++);
        delayMs(1000);
    }
}
#endif

#ifdef TEST_ZM_RESET
/** This utility will toggle the reset line of the module and determine whether the proper firmware 
is loaded. When using SPI, toggling the reset line should cause SRDY to go high after approximately
400mSec to indicate a SYS_RESET_IND message.
This utility doesn't read the SPI bytes, just checks the SRDY line. 
Verify that the pin connected to the hardware reset of the Module is pulled down for 1mSec.
Configure logic analyzer to trigger on a 1->0 transition on Module reset line */
int main( void )
{
    halInit();
    printf("\r\nTest Zigbee Module Reset\r\n");
    RADIO_OFF();
    delayMs(1);
    RADIO_ON();
    
#define TEST_SRDY_INTERVAL_MS           10        // check SRDY every 10 mSec
#define TEST_SRDY_TIMEOUT_MS            2000      // ...and timeout after 2000mSec
#define TEST_SRDY_MINIMUM_TIMEOUT_MS    300       // If SRDY transitions less then this than an error.
    unsigned int elapsedTime = 0;       //now, poll for SRDY going low...
    do
    {
        delayMs(TEST_SRDY_INTERVAL_MS);
        elapsedTime += TEST_SRDY_INTERVAL_MS;
    }
    while ((elapsedTime < TEST_SRDY_TIMEOUT_MS) && (SRDY_IS_HIGH()));
    if ((SRDY_IS_LOW()) && (elapsedTime > TEST_SRDY_MINIMUM_TIMEOUT_MS))
    {
        printf("Test PASSED - SRDY went low after approximately %umS\r\n", elapsedTime);
    }
    else
    {
        printf("ERROR - SRDY never went low, or went low in less than %umS\r\n", TEST_SRDY_MINIMUM_TIMEOUT_MS);
    }
}
#endif


#ifdef TEST_SPI_WRITE
/** Use this to verify that the SPI port is configured correctly and writing bytes
Use a logic analyzer or scope and view the bytes being written.
The ZM won't do anything but you'll be able to determine whether the SPI port is working.
Configure logic analyzer to trigger on a 1->0 transition on CS.
Verify that you're seeing the bytes written out the SPI port.
*/
int main( void )
{
    halInit();                          //Initialize hardware    
    halSpiInitModule();
    printf("\r\nTest SPI Write. First Reset the Module\r\n");
    RADIO_OFF();
    delayMs(1);
    RADIO_ON();
#define TEST_SRDY_INTERVAL_MS 10  //check SRDY every 10 mSec
#define TEST_SRDY_TIMEOUT_MS  1000
    unsigned int elapsedTime = 0;       //now, poll for SRDY going low...
    do
    {
        delayMs(TEST_SRDY_INTERVAL_MS);
        elapsedTime += TEST_SRDY_INTERVAL_MS;
    }
    while ((elapsedTime < TEST_SRDY_TIMEOUT_MS) && (SRDY_IS_HIGH()));
    if (SRDY_IS_LOW())
    {
        printf("Test PASSED - SRDY went low after approximately %umS\r\n", elapsedTime);
    }
    else
    {
        printf("ERROR - SRDY never went low\r\n");
    }
    
    printf("Now trying to write via SPI.\r\n");
#define TEST_DATA {0x05, 0x04, 0x03, 0x02, 0x01, 0x00}
#define TEST_DATA_LENGTH 6
    unsigned char test[] = TEST_DATA;
    spiWrite(test, TEST_DATA_LENGTH);
    printf("Done!\r\n");
}
#endif

/** If either TEST_SYS_RESET_IND or TEST_SYS_GET_MAC then we'll need the following functionality. 
These are repeated elsewhere in the library but placed here to avoid circular dependencies. */
#if defined(TEST_SYS_RESET_IND) || defined(TEST_SYS_GET_MAC)

unsigned char zmBuf[0xFF];

/** Send a Synchronous Request */
signed int sendSreq()
{
  SPI_SS_SET();   
  while (SRDY_IS_HIGH()) ;                    //wait until SRDY goes low
  spiWrite(zmBuf, (*zmBuf + 3));              // *bytes (first byte) is length after the first 3 bytes, all frames have at least the first 3 bytes
  *zmBuf = 0; *(zmBuf+1) = 0; *(zmBuf+2) = 0; //poll message is 0,0,0
  //NOTE: MRDY must remain asserted here, but can de-assert SS if the two signals are separate
  
  //Now: Data was sent, wait for Synchronous Response (SRSP)
  while (SRDY_IS_LOW()) ;                     //wait for data
  //NOTE: if SS & MRDY are separate signals then can re-assert SS here.
  spiWrite(zmBuf, 3);
  if (*zmBuf > 0)                             // *bytes (first byte) contains number of bytes to receive
    spiWrite(zmBuf+3, *zmBuf);              //write-to-read: read data into buffer    
  SPI_SS_CLEAR();                             // re-assert MRDY and SS
  return 0;  
}

/** Poll the module */
signed int spiPoll()
{
    *zmBuf = 0; *(zmBuf+1) = 0; *(zmBuf+2) = 0;  //poll message is 0,0,0 
    return(sendSreq());
}
#endif

#ifdef TEST_SYS_RESET_IND
/** Reset the Module, poll for the SYS_RESET_IND message and display the contents. */
int main( void )
{
    halInit();                          //Initialize hardware
    halSpiInitModule();
    while (1)
    {
        printf("\r\nResetting Module and displaying results:\r\n");
        RADIO_OFF();
        delayMs(1);
        RADIO_ON();
#define INITIAL_DELAY
        delayMs(100);                        //Necessary to allow proper module startup        
#define TEST_SYS_RESET_IND_INTERVAL_MS 10  //check SRDY every 10 mSec
#define TEST_SYS_RESET_IND_TIMEOUT_MS  3000
    unsigned int elapsedTime = 0;       //now, poll for SRDY going low...
    do
    {
        delayMs(TEST_SYS_RESET_IND_INTERVAL_MS);
        elapsedTime += TEST_SYS_RESET_IND_INTERVAL_MS;
    }
    while ((elapsedTime < TEST_SYS_RESET_IND_TIMEOUT_MS) && (SRDY_IS_HIGH()));
    if (SRDY_IS_LOW())
    {
        printf("Test PASSED - SRDY went low after approximately %umS\r\n", elapsedTime+100);
    }
    else
    {
        printf("ERROR - SRDY never went low\r\n");
    }
    printf("Now polling module for reset message.\r\n");
        spiPoll();
        for (int i=0; i< (zmBuf[0] + 3); i++)
            printf("%02X ", zmBuf[i]);
        printf("\r\n");
        
        delayMs(1000);
    }
}
#endif

#ifdef TEST_SYS_GET_MAC
/** Reset the module and then get the MAC. */
int main( void )
{
    halInit();                          //Initialize hardware
    halSpiInitModule();
    while (1)
    {
        printf("Resetting Module to get MAC address\r\n");
        RADIO_OFF();
        delayMs(1);
        RADIO_ON();
        delayMs(10);                        //Necessary to allow proper module startup    
        spiPoll();
        printf("Rx: ");
        for (int i=0; i< (zmBuf[0] + 3); i++)
            printf("%02X ", zmBuf[i]);
        printf("\r\n");
        
        zmBuf[0] = 1;      //ZB_GET_DEVICE_INFO_PAYLOAD_LEN;
        zmBuf[1] = 0x26;   //MSB(ZB_GET_DEVICE_INFO);
        zmBuf[2] = 0x06;   //LSB(ZB_GET_DEVICE_INFO);
        zmBuf[3] = 1;      //DIP_MAC_ADDRESS;
        //Note: response will be (hex) 09 66 06 <8bytes of MAC>
        sendSreq();
        printf("MAC Address, LSB first:");
        for (int i=4; i< (zmBuf[0] + 3); i++)
            printf("%02X ", zmBuf[i]);
        printf("\r\n");
        delayMs(1000);
    }
}
#endif


#ifdef TEST_UART_INTERRUPT
//Note: this test only relates to the debug UART interface, not the module
extern void (*debugConsoleIsr)(int8_t);

void debugIsr(int8_t c)
{
    toggleLed(1);
    printf("%c", c);
}

int main( void)
{
    halInit();                          //Initialize hardware  
    debugConsoleIsr = &debugIsr;
    HAL_ENABLE_INTERRUPTS();  
    printf("\r\nTest UART Interrupt. Press any key, and LED should toggle.\r\n");
    while(1)
    {
        printf(".");
        delayMs(1000);
    }   
}
#endif

//
//  Module Serial Interface
//

#if defined(TEST_SERIAL_ZM_RESET) || defined(TEST_SERIAL)

void auxSerialPortHandler(char c)
{
    RTS_OFF();   // stop bytes coming from module with flow control
    printf("%02X ", c);
    RTS_ON();    // resume flow
}

extern void (*auxSerialPort)(char);

#endif

#ifdef TEST_SERIAL_ZM_RESET
int main( void )
{
    halInit();
    auxSerialPort = &auxSerialPortHandler;
    HAL_ENABLE_INTERRUPTS();

    printf("\r\n\r\n");
    for (int i=0; i<40; i++)
        printf("*");
    printf("\r\nTEST_SERIAL_ZM_RESET\r\n");

    RADIO_OFF();
    delayMs(10);
    RADIO_ON();
    //delayMs(100);
    //putcharAux(0x07);  //cut through bootloader
    //delayMs(100);
        //halEnableAuxFlowControl();
    while (1)
    {
        printf(".");
        delayMs(1000);
    }
}
#endif


#ifdef TEST_SERIAL
/** Reset the module and then get the MAC. */
//
// Start of Circular Buffer
//
#define MESSAGE_BUFFER_SIZE 0xFF 
uint8_t size = 0;
uint8_t messageBuffer[MESSAGE_BUFFER_SIZE];
uint8_t writePointer = 0;
uint8_t readPointer = 0;
volatile uint8_t inUse  = 0;

// returns 1 if buffer is full, 0 if buffer is not full
int bufferFull(void)  { return (readPointer == writePointer) && (size == MESSAGE_BUFFER_SIZE); }

void initializeQueue() {  size = 0; writePointer = 0; readPointer = 0; inUse = 0; }

// returns 1 if buffer is empty, 0 if buffer is not empty
int bufferEmpty(void) { return (readPointer == writePointer) && (size == 0); }

// push char onto queue
void push(char c)
{
    if (++writePointer >= MESSAGE_BUFFER_SIZE)     // increase write_pointer, check if at end of array
        writePointer = 0;
    messageBuffer[writePointer] = c;    
    size++;
}

// pull char from queue
char pull(void) 
{  
    if (++readPointer >= MESSAGE_BUFFER_SIZE) 
        readPointer = 0;
    
    //printf("\nPopped char %c", buffer[readPointer]);
    
    // enter space on place of read char so we can see it is removed
    //messageBuffer[readPointer] = 0x20; 
    size--;
    return messageBuffer[readPointer];
}
//
// End of Circular Buffer
//

/** Sends a test string to the module. The message will write config to be a coordinator.
In response you should receive a response of 0xFE 0x01 0x66 0x05 0x00 0x62 message.*/
void sendTestMessage()
{
    printf("Send Test Message\r\n");
#define TEST_MESSAGE_LENGTH 8
    unsigned char zmBuf[TEST_MESSAGE_LENGTH];
    
    zmBuf[0] = 0xFE;
    zmBuf[1] = 0x03;      
    zmBuf[2] = 0x26;  
    zmBuf[3] = 0x05;  
    zmBuf[4] = 0x87;
    zmBuf[5] = 0x01;
    zmBuf[6] = 0x00;
    zmBuf[7] = 0xA6;
    //unsigned char fcs = calcFCS(zmBuf, 8);
    
    for (int i=0; i< TEST_MESSAGE_LENGTH; i++)
        putcharAux(zmBuf[i]);
}

extern void (*auxSerialPort)(char);

int main( void )
{
    halInit();
    auxSerialPort = &auxSerialPortHandler;
    HAL_ENABLE_INTERRUPTS();
    //resetMessage();
    
    printf("\r\n");
    for (int i=0; i<40; i++)
        printf("*");
    printf("\r\nTest Serial\r\n");
    
    int count = 0, cycle = 0;
    
    //while (1)
    //{
    //printf("\r\nTesting Serial Connection to Module %u\r\n", count++);
    
    //Reset the module using hardware reset line. We should then receive a SYS_RESET_IND message in approx 700mSec
    
    RADIO_OFF();
    delayMs(1);
    RADIO_ON();
    delayMs(1);
    //putcharAux(0x07);  //cut through bootloader
    delayMs(100);
    /*
    while (1)
    {
        printf(".");
        delayMs(1000);
        //if (!(bufferEmpty()))
        //  printf("%02X ", pull());
    }
    */
    //Wait for SYS_RESET_IND Message to be displayed
    for (int i=0; i<5; i++)
    {
        printf(".");
        delayMs(1000);
    }
    
    
    
    
    
#define CHECK_MESSAGE_COMPLETE_INTERVAL_MS  10 //how often we check message complete
#define CHECK_MESSAGE_COMPLETE_TIMEOUT_MS   2000  //if no message received in 2000mSec then error
#define CHECK_MESSAGE_CYCLES (CHECK_MESSAGE_COMPLETE_TIMEOUT_MS / CHECK_MESSAGE_COMPLETE_INTERVAL_MS)
    
    //Now, wait for response from module and keep track of how long it takes
    /*
    cycle = 0;
    do {
    delayMs(CHECK_MESSAGE_COMPLETE_INTERVAL_MS);
    cycle++;
}
    while ((!(messageBufferState == COMPLETE)) && (cycle < CHECK_MESSAGE_CYCLES));
    if (cycle == CHECK_MESSAGE_CYCLES)
    {
    printf("ERROR - SYS_RESET_IND message not received in %umSec\r\n", CHECK_MESSAGE_COMPLETE_TIMEOUT_MS);
} else {
    printf("SYS_RESET_IND message received in %umSec\r\n", cycle*CHECK_MESSAGE_COMPLETE_INTERVAL_MS);
}
    displayMessageBuffer();
    resetMessage();
    */
    
    //Send a test message to the module and receive a response:
    printf("Sending Test Message - you should see a response of: FE 01 66 05 00 62 returned\r\n");
    sendTestMessage();
        //Wait for Response Message to be displayed
    for (int i=0; i<5; i++)
    {
        printf(".");
        delayMs(1000);
    }
    
    //Now, check that we receive a response -  we should get a response in approx. 20mSec.
    /*
    cycle = 0;
    do {
    delayMs(CHECK_MESSAGE_COMPLETE_INTERVAL_MS);
    cycle++;
}
    while ((!(messageBufferState == COMPLETE)) && (cycle < CHECK_MESSAGE_CYCLES));
    if (cycle == CHECK_MESSAGE_CYCLES)
    {
    printf("ERROR - message not received in %umSec\r\n", CHECK_MESSAGE_COMPLETE_TIMEOUT_MS);
} else {
    printf("message received in %umSec\r\n", cycle*CHECK_MESSAGE_COMPLETE_INTERVAL_MS);
}
    displayMessageBuffer();
    resetMessage();      
    
    delayMs(3000);
}
    */
    
}




#endif

#ifdef TEST_TRISTATE_ALL_PORTS
int main( void)
{
    oscInit(); 
    halSetAllPinsToInputs();
    
    while(1)
    {
        volatile int i = 0xFFFF;
        while (i--);
    }   
}


#endif


/* @} */
