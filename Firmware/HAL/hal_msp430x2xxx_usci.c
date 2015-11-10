/**
* @file hal_msp430x2xxx_usci.c
*
* @brief Hardware Abstraction Layer (HAL) for the TI MSP430F2xxx series of microcontrollers.
*
* This file contains generic utilities for using this processor.
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

#include "hal_msp430x2xxx_usci.h"

/** Configures the selected port.
Also enables the Rx interrupt for this UART.
@param port must be USCIA0 or USCIA1
@param baudRate must be BAUD_RATE_9600, BAUD_RATE_19200, BAUD_RATE_38400, or BAUD_RATE_115200
@pre SMCLK is 4MHz
@see Table 15-5 of MSP430F2xxx Family User's Guide, slau144
@return 0 if success, else an error
*/
signed int halUartInit(unsigned char port, unsigned char baudRate)
{ 
    switch (port)
    {
    case USCIA0:
          UCA0CTL1 = UCSWRST;                     // **Stop USCI state machine**  
        switch (baudRate)
        {
        case BAUD_RATE_9600:
            UCA0BR0 = 26; UCA0BR1 = 0;                
            UCA0MCTL = UCBRS_0 + UCBRF_1 + UCOS16;                   
            break;     
        case BAUD_RATE_19200:
            UCA0BR0 = 13; UCA0BR1 = 0;                
            UCA0MCTL = UCBRS_0 + UCBRF_0 + UCOS16;    
            break;    
        case BAUD_RATE_38400:
            UCA0BR0 = 6; UCA0BR1 = 0;                 
            UCA0MCTL = UCBRS_0 + UCBRF_8 + UCOS16;   
            break;      
        case BAUD_RATE_115200:
            UCA0BR0 = 2; UCA0BR1 = 0;                  
            UCA0MCTL = UCBRS_3 + UCBRF_2 + UCOS16;           
            break;         
        default:
            return ERROR_HAL_INVALID_BAUD_RATE;
        }
        UCA0CTL1 |= UCSSEL_2;                     // SMCLK
        UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt  
        break;
        
    case USCIA1:
          UCA1CTL1 = UCSWRST;                     // **Stop USCI state machine**  
        switch (baudRate)
        {
        case BAUD_RATE_19200:
            UCA1BR0 = 13; UCA1BR1 = 0;                // 4mHz smclk w/modulation, table 15-5 (UCBRx=13, UCBRSx=0, UCBRFx=0)
            UCA1MCTL = UCBRS_0 + UCBRF_0 + UCOS16;   // Modulation UCBRSx=1, over sampling         
            break;
        case BAUD_RATE_9600:
            UCA1BR0 = 26; UCA1BR1 = 0;                // 4mHz smclk w/modulation, table 15-5 (UCBRx=26, UCBRSx=0, UCBRF=1)
            UCA1MCTL = UCBRS_0 + UCBRF_1 + UCOS16;   // Modulation UCBRSx=1, over sampling         
            break;      
        case BAUD_RATE_115200:
              UCA1BR0 = 34; UCA1BR1 = 0;                 // 4mHz smclk w/modulation for 115kbps, table 15-4 
              UCA1MCTL = UCBRS_6 + UCBRF_0;   // Modulation, NO over sampling  
            break;         
        default:
            return ERROR_HAL_INVALID_BAUD_RATE;
        }
        UCA1CTL1 |= UCSSEL_2;                     // SMCLK
        UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
        UC1IE |= UCA1RXIE;                          // Enable USCI_A1 RX interrupt
        break;
        
    default:  //bad port
        return ERROR_HAL_INVALID_USCI_PORT;
    }
    return 0;
}

/** Send one character out the specified port. Used in putchar(). 
Note, in putchar, must convert to unsigned char, e.g.
int putchar(int b)
{
halUartCharPut(p, (unsigned char) (b & 0xFF));
return b;
}
@param port must be USCIA0 or USCIA1
@param the data to send
@return the data if success, else ERROR_HAL_INVALID_USCI_PORT if bad port.
*/
signed int halUartCharPut(unsigned char port, unsigned char data)
{
    switch (port)
    {
    case USCIA0:
        while (!(IFG2 & UCA0TXIFG));   // Wait for ready
        UCA0TXBUF = data;  
        return data;
        
    case USCIA1:
        while (!(UC1IFG&UCA1TXIFG));               // USCI_A1 TX buffer ready?
        UCA1TXBUF = data;  
        return data;
        
    default:  //error
        return ERROR_HAL_INVALID_USCI_PORT;
    }
}

/** Configures the selected SPI port.
* @param port must be USCIB0
* @note CC2530 SPI clock speed < 4MHz. SPI port configured for clock polarity of 0, clock phase of 0, and MSB first.
* @note On MDB the RFIC SPI port is USCIB1
* @note Modify this method for other hardware implementations.
* @pre SPI pins configured correctly: Clock, MOSI, MISO configured as SPI function; Chip Select configured as an output; SRDY configured as an input.
* @post SPI port is configured for RFIC communications.
* @note immediately after calling this method you should call SPI_SS_CLEAR()
* @todo implement baudRate
*/
signed int halSpiInit(unsigned char port, unsigned char baudRate)
{ 
    if (port != USCIB1)
        return ERROR_HAL_INVALID_USCI_PORT;
    
    UCB1CTL1 |= UCSSEL_2 | UCSWRST;                 //serial clock source = SMCLK, hold SPI interface in reset
    UCB1CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC;     //clock polarity = inactive is LOW (CPOL=0); Clock Phase = 0; MSB first; Master Mode; Synchronous Mode
    UCB1BR0 = 2;  UCB1BR1 = 0;                      //SPI running at 2MHz (SMCLK / 2)
    UCB1CTL1 &= ~UCSWRST;                           //start USCI_B1 state machine
    //SPI_SS_CLEAR(); 
    return 0;
}

/**
* Sends a message over SPI on USCI_B1. 
* To read data out, you must write data in.
* To Write, set *bytes, numBytes.
* To Read, set *bytes only. Don't need to set numBytes because CC2530ZNP will stop when no more bytes read.
* @param bytes the data to be sent or received.
* @param numBytes the number of bytes to be sent. This same buffer will be overwritten with the received data.
* @note Modify this method for other hardware implementations.
* @pre SPI port configured for writing
* @pre Peripheral has been initialized
* @post bytes contains received data, if any
* @return 0 if success, -1 if incorrect port
* @note - chip select should be controlled in the wrapping function with SPI_SS_SET() and SPI_SS_CLEAR()
*/
signed int halSpiWrite(unsigned char port, unsigned char *bytes, unsigned char numBytes)
{
    if (port != USCIB1)
        return ERROR_HAL_INVALID_USCI_PORT;
    //SPI_SS_SET(); 
    while (numBytes--)
    {  
        UCB1TXBUF = *bytes;
        while (!(UC1IFG & UCB1RXIFG)) ; //WAIT for a character to be received, if any
        *bytes++ = UCB1RXBUF;  //read bytes
    }
    //SPI_SS_CLEAR();
    return 0;
}




