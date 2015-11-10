/**
* @file zm_phy_uart.c
*
* @brief Physical Interface Layer to the Module using the Universal Asynchronous 
* Receiver/Transmitter interface.
* This file acts as an interface between the module library methods and the hardware (hal file).
* 
* @note To output SPI transmit information, define ZM_PHY_UART_VERBOSE.
* It is recommended to define this on a per-project basis. 
* In IAR, this can be done in Project Options : C/C++ compiler : Preprocessor
* In the defined symbols box, add:
* ZM_PHY_UART_VERBOSE
*
* @section Troubleshooting
* Module UART requires 115,200 kbs, with flow control enabled. Most problems with
* the UART interface are caused by flow control errors, so check these first.
*
* @section Concurrency
* To prevent code in two places accessing the messageBuffer at the same time, we use a mutex.
* This mutex is implemented as a bit flag in the messageFlags variable. Accessing the mutex is through
* a spinlock, which waits until the buffer becomes available before using it. For example:
<pre>
  while (messageFlags & MESSAGE_BUFFER_IN_USE);   //wait for msgBuffer to be available
  messageFlags |= MESSAGE_BUFFER_IN_USE;          //lock the buffer
</pre>
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

#include "../HAL/hal.h"
#include "zm_phy_uart.h"
#include "module_errors.h"
#include "../Common/utilities.h"
#include <stdint.h>
#include <string.h>

#define MODULE_BUFFER_SIZE  0xFF

/** This buffer will hold the transmitted messages and received SRSP Payload after sendMessage() was 
called. If fragmentation support is not required then this can be smaller, e.g. 100B.*/
uint8_t zmBuf[MODULE_BUFFER_SIZE];

/** Function pointer for the ISR called when a byte is received on the serial 
port connected to the module. This must be defined in the hal file. */
extern void (*auxSerialPort)(char);  //only used for UART interface

//
//  UART SPECIFIC DEFINES
//

/** Used to indicate an entire message has arrived. Set by ISR, cleared by main method */
volatile uint8_t messageFlags = 0;

//
// UART specific variables, protected by messageFlags.MESSAGE_BUFFER_IN_USE mutex:
//
uint8_t messageBufferIndex = 0;
uint8_t messageBuffer[MESSAGE_BUFFER_SIZE];  //Used for receiving UART messages
uint8_t fcs = 0;  //frame check sequence

/** 
The states of the messageBuffer receive state machine. This is private and
should not be needed to be accessed outside this file */
enum messageBufferStates {
  /** awaiting the start of a new message */
  NOT_STARTED,
  /** a message has started, but not yet completed */
  STARTED,
  /** all the bytes of message body received, need to check FCS */
  AWAITING_FCS,
  /** FCS check complete, message can now be read */
  COMPLETE,
  /** there was an error in parsing */
  ERROR          
} messageBufferState;

/* Initializes the module PHY interface.
*/
void zm_phy_init()
{
  auxSerialPort = &auxSerialPortHandler;  //point the ISR function pointer to our method
}

/** 
Indicates whether the module has a message ready to be processed.
@return true (1) if there is a complete message in messageBuffer ready for processing, or 0 otherwise.
*/
uint8_t moduleHasMessageWaiting()
{
  return (messageBufferState == COMPLETE);
}

/** 
Clears the messageFlags and sets messageBufferIndex back to zero. After this method is called we are 
ready to receive a new message from the Module.
*/
void resetMessage()
{
  messageFlags |= MESSAGE_BUFFER_IN_USE; //lock the buffer with the mutex
  messageBufferIndex = 0;
  messageBufferState = NOT_STARTED;
  fcs = 0;
  messageFlags = 0; //Note: this clears the mutex too.  
}

/** 
Processes bytes received from the Module. Should be called in the serial port ISR for the module 
serial port. For example:
<pre>
#pragma vector=USCIAB1RX_VECTOR
__interrupt void USCI1RX_ISR(void)
{
  if (UC1IFG & UCA1RXIFG)
  {
    auxSerialPort(UCA1RXBUF);    //reading this register clears the interrupt flag                  
  }
}
</pre>
Message processing is handled according to a simple state machine. If a message 
has already started, then append bytes to messageBuffer. When a byte is received:
1. Assert RTS so that the module doesn't send another byte while we're processing this one
2. Set messageFlags.MESSAGE_BUFFER_IN_USE. This is a simple mutex to control 
access to messageBufferIndex, messageBuffer, and fcs to prevent concurrency issues
3. Process received byte based on the state of messageBufferState
4. Clear messageFlags.MESSAGE_BUFFER_IN_USE mutex
5. Deassert RTS to allow more bytes to be received
@param c the byte that was received in the ISR.
*/
void auxSerialPortHandler(char c)
{
  RTS_OFF();   //stop bytes coming from module with flow control
  //do we need to check mutex in ISR?
  while (messageFlags & MESSAGE_BUFFER_IN_USE);  //spinlock: wait for msgBuffer to be available   
  //non spinlock implementation: messageFlags |= MESSAGE_BUFFER_IN_USE; //lock the buffer
  
  switch (messageBufferState)
  {
  case NOT_STARTED:
    if (c == UART_START_OF_FRAME)  //mew message
    {
      messageBufferState = STARTED;
      break;
    }
  case STARTED:
    if (messageBufferIndex < MESSAGE_BUFFER_SIZE)
    {
      messageBuffer[messageBufferIndex] = c;  //write byte to message buffer
      messageBufferIndex++;
      if (messageBufferIndex == (messageBuffer[UART_MESSAGE_LENGTH_FIELD] + MESSAGE_OVERHEAD))
        messageBufferState = AWAITING_FCS;  //next byte will be FCS
      fcs ^= c;
    } else {      //buffer overrun; discard the contents and reset the message.
      messageBufferState = ERROR;
    }
    break;
  case AWAITING_FCS:
    if (c == fcs)
    {
      messageBufferState = COMPLETE;
    } else {
      messageBufferState = ERROR;  //FCS check failed
    }
    break;
  case COMPLETE:
    //error - we received a byte with the messageBuffer still busy.
    // *fall-through* (no break here)
  default:
    //error - should never happen
    messageBufferState = ERROR;
    break;
  }
  
  if (messageBufferState == ERROR)
    resetMessage();
  
  messageFlags &= ~MESSAGE_BUFFER_IN_USE; //unlock the buffer
  RTS_ON();
}


/**
Sends a Module Synchronous Request (SREQ) message and retrieves the response. A SREQ is a message to 
the Module that is immediately followed by a Synchronous Response (SRSP) message from the Module. 
As opposed to an Asynchronous Request (AREQ) message, which does not have a SRSP. This is a private 
method that gets wrapped by sendMessage().
@pre Module has been initialized
@pre zmBuf contains a properly formatted message. No validation is done.
@post received data is written to zmBuf
*/
moduleResult_t sendSreq()
{  
  /* Frame Check Sequence - this will be computed on the fly, as each byte is
  received. It's just a simple XORing of all received bytes */
  uint8_t fcs = 0;                     
  
  /* The total message length, excluding start of frame and FCS. */
  uint8_t messageLength = *zmBuf + 3;   
  
  putcharAux(UART_START_OF_FRAME);      // Send the SOF (0xFE)
  
  int i;
  for (i=0; i< messageLength; i++)  // Now, for all bytes in the message:
  {
    putcharAux(zmBuf[i]);       // Send the byte
    fcs ^= zmBuf[i];            // Compute FCS on this byte
  }
  
  putcharAux(fcs);  //The message body has been sent. now send FCS
  
  /* Now await the reply. Most of this will be handled by the ISR so we need to 
  wait until the messageBufferState (set in ISR) indicates a message is ready. 
  Most responses are fast - 20mSec or less, but zdoStartApplication requires 
  approximately 400mSec.*/
#define CHECK_MESSAGE_COMPLETE_INTERVAL_MS  2 //how often we check message complete
#define CHECK_MESSAGE_COMPLETE_TIMEOUT_MS   2000  //how long to wait
  
  return receiveAreq(CHECK_MESSAGE_COMPLETE_INTERVAL_MS, CHECK_MESSAGE_COMPLETE_TIMEOUT_MS);
}

/** 
Get a message that is in messageBuffer and copy to zmBuf for further processing.
@pre  moduleHasMessageWaiting() is true
@post zmBuf holds the message that was in messageBuffer
*/
moduleResult_t getMessage()
{
  if (messageBufferState == COMPLETE)
  {
    memcpy(zmBuf, messageBuffer, messageBufferIndex);    //now copy the received message into zmBuf
    resetMessage();
    return MODULE_SUCCESS;
  } else {
    return ZM_PHY_OTHER_ERROR;    
  }
}

/** Wait to receive an asynchronous message. Similar to spiPoll() but with wait.
@param intervalMs how often to check that we received the message.
@param timeoutMs how long to wait, total, for the message
@return MODULE_SUCCESS if message received before timeoutMs or else ZM_PHY_SRSP_TIMEOUT if no 
message was received before timeout.
@post received data is written to zmBuf and messageBuffer is reset
*/
moduleResult_t receiveAreq(uint8_t intervalMs, int16_t timeoutMs)
{
#ifdef ZM_PHY_UART_VERBOSE    
  printf("Waiting to receive message; interval=%umSec, timeout=%umSec\r\n", intervalMs, timeoutMs);
#endif     
  do {                // check to see if a message was received
    delayMs(intervalMs);
    timeoutMs -= intervalMs;
  }
  while ((!(messageBufferState == COMPLETE)) && (timeoutMs > 0));
  
  if (timeoutMs > 0)  // received a message before timeout
  {
#ifdef ZM_PHY_UART_VERBOSE    
    printf("Success - response received %umSec before timeout\r\n", timeoutMs);
    displayMessageBuffer();
#endif     
    memcpy(zmBuf, messageBuffer, messageBufferIndex);    //now copy the received message into zmBuf
    resetMessage(); 
    return MODULE_SUCCESS;  
  } else {          // did not receive a message before timeout
#ifdef ZM_PHY_UART_VERBOSE  
    printf("Timeout");
    displayMessageBuffer();    
#endif         
    resetMessage(); 
    return ZM_PHY_SRSP_TIMEOUT;
  }
}

/** 
Public method to send messages to the Module. This will send one message and then receive the 
Synchronous Response (SRSP) message from the Module to indicate the command was received.
@pre zmBuf contains a properly formatted message
@pre Module has been initialized
@post buffer zmBuf contains the response (if any) from the Module. 
*/
moduleResult_t sendMessage()
{
#ifdef ZM_PHY_UART_VERBOSE    
  printf("Tx: ");
  printHexBytes(zmBuf, zmBuf[0] + 3);
#endif    
  
  uint8_t expectedSrspCmdMsb = zmBuf[1] + SRSP_OFFSET;    //store these so we can compare with what is returned
  uint8_t expectedSrspCmdLsb = zmBuf[2];
  
  moduleResult_t result = sendSreq();                     //send message, buffer now holds received data
  
  if (result != MODULE_SUCCESS)                           //ERROR - sendSreq() timeout
  {
#ifdef ZM_PHY_UART_VERBOSE_ERRORS    
    printf("ERROR - sreq() timeout %02X\r\n", result);
#endif 
    return result;
  }
  
  /* The correct SRSP will always be 0x4000 + cmd, or simpler 0x4000 | cmd
  For example, if the SREQ is 0x2605 then the corresponding SRSP is 0x6605 */
  if ((zmBuf[SRSP_CMD_MSB_FIELD] == expectedSrspCmdMsb) && (zmBuf[SRSP_CMD_LSB_FIELD] == expectedSrspCmdLsb))    //verify the correct SRSP was received
  {
    return MODULE_SUCCESS;
  } else {
#ifdef ZM_PHY_UART_VERBOSE_ERRORS    
    printf("ERROR - Wrong SRSP - received %02X-%02X, expected %02X-%02X\r\n", zmBuf[1], zmBuf[2],expectedSrspCmdMsb,expectedSrspCmdLsb);
#endif          
    return ZM_PHY_INCORRECT_SRSP;   //Wrong SRSP received
  }
}


/** 
Displays the contents of the messageBuffer. Locks the messageBuffer while reading to
prevent a concurrency problem. 
@note If you have sufficient memory and the printf is slow then it would be better to copy 
messageBuffer to a new buffer and then printf that. Or use a circular buffer on the debug UART.
*/
void displayMessageBuffer()
{
  while (messageFlags & MESSAGE_BUFFER_IN_USE);   //wait for msgBuffer to be available
  messageFlags |= MESSAGE_BUFFER_IN_USE;          //lock the buffer
  printf("messageFlags = 0x%02X, fcs=0x%02X, messageBufferIndex = %u\r\n", messageFlags, fcs, messageBufferIndex);
  printHexBytes(messageBuffer, messageBufferIndex);
  //for (int i=0; i<messageBufferIndex; i++)
  //  printf("%02X ", messageBuffer[i]);
  //printf("\r\n");
  messageFlags &= MESSAGE_BUFFER_IN_USE;          //unlock the buffer
}