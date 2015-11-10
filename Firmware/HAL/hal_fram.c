/**
* @file hal_fram.c
*
* @brief Hardware Abstraction Layer (HAL) for the TI MSP-EXP430FR5739 and Anaren Zigbee BoosterPack
*
* This file must be modified if changing hardware platforms.
*
* The Zigbee Module library & examples require several methods to be defined. 
* See hal_helper documentation. Also see hal_fram.h for macros that must be defined.
*
* @see hal_helper.c for utilities to assist when changing hardware platforms
* @see "Migrating from the MSP430F2xx Family to the MSP430FR57xx Family" (slaa499) from TI
* @see "Migrating from the USCI Module to the eUSCI Module" (slaa522) from TI
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

@section CC2530DK connection notes
<pre>
P1 on CC2530DK connects to RF1 on FRAM board
P2 on CC2530DK connects to RF2 on FRAM board
P1/RF1 pins:
Pin CC2530  Name    FRAM
1   GND             GND
2   n/c     
3   ccP0.4  SRDY    mspP2.4
4   ccP1.3  EN(N/C)  
5   ccP0.1  GPIO1   mspP1.0
6   ccP1.0  GPIO3   mspP1.1
7   ccP0.2          mspP1.1
8   N/C             mspP1.2
9   ccP0.3  MRDY    mspP1.2 (duplicate)
10  ccP2.1          mspP4.1
11  ccP0.0  GPIO0
12  ccP2.2          mspP2.3
13  ccP1.1  PAEN(N/C)  
14  ccP1.4  SS      mspP1.3 UCB0STE
15  ccP0.6  GPIO2
16  ccP1.5  SCLK    mspP2.2 UCB0CLK
17  ccP0.7  HGM(N/C)  
18  ccP1.6  MOSI    mspP1.6 UCB0SIMO
19  GND             GND
20  ccP1.7  MISO    mspP1.7 UCB0SOMI

P2/RF2
Pin CC2530          FRAM
1             
2                     GND
3
4
5
6
7   VCC             RFPWR
8
9   VCC             RFPWR
10  
11
12
13                  mspP2.0
14  
15  ccRST   reset   mspP1.0 (duplicate)
16  
17  ccP1.2  CFG0
18  ccP0.5          mspP2.7
19  ccP2.0  CFG1    mspP4.0   Tie Hi for SPI
20                  mspP3.7
</pre>
*/

#include "hal_fram.h"
#include <stdint.h>

/** This is a function pointer for the Interrupt Service Routine called when a debug console character is received.
To use it, declare it with
<code> extern void (*debugConsoleIsr)(char);  </code> 
and then point it to a function you created, e.g.
<code> debugConsoleIsr = &handleDebugConsoleInterrupt;  </code>
and your function handleDebugConsoleInterrupt() will be called when a byte is received on the debug serial port.
*/
void (*debugConsoleIsr)(int8_t);

/** Function pointer for the ISR called when the button is pressed. 
Parameter is which button was pressed. */
void (*buttonIsr)(int8_t);

/** Function pointer for the ISR called when a timer generates an interrupt*/
void (*timerIsr)(void);

/** Function pointer for the ISR called when a SRDY interrupt occurs*/
void (*srdyIsr)(void);

/** Flags to indicate when to wake up the processor. These are read in the various ISRs. 
If the flag is set then the processor will be woken up with HAL_WAKEUP() at the end of the ISR. 
This is required because HAL_WAKEUP() cannot be called anywhere except in an ISR. */
uint16_t wakeupFlags = 0;

/** The post-calibrated frequency of the Very Low Oscillator (VLO). 
This MUST be calibrated by calibrateVlo() prior to use.
Set with calibrateVlo() and read by initTimer(). */
uint16_t vloFrequency = 0;

 
/** 
Debug console interrupt service routine, called when a byte is received on USCIA0. 
*/
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    if (UCA0IV & 0x02)               //debug console character received
    {
        toggleLed(0);
        debugConsoleIsr(UCA0RXBUF);    //reading this register clears the interrupt flag
    } 
}

/** 
Port 4 interrupt service routine, called when an interrupt-enabled pin on port 4 changes state. 
*/
#pragma vector=PORT4_VECTOR
__interrupt void PORT4_ISR(void)
{
    if (P4IFG & BIT1)                   //Modify this based on which pin is connected to Button
    {
        buttonIsr(0);   // Button 0 was pressed
        //if (wakeupFlags & WAKEUP_AFTER_BUTTON)    
        //    HAL_WAKEUP();   
    }
    P4IFG = 0;                          // clear the interrupt
}

/** 
Simple placeholder to point the function pointers to so that they don't cause mischief 
*/
void doNothing(int8_t a)
{
    __delay_cycles(1);  //prevent this from getting optimized out
}

/** 
Initializes Oscillator: turns off WDT, configures MCLK = 8MHz using internal DCO & sets SMCLK = 4MHz.
@see MSP430FR57xx_UCS_01.c
*/
void oscInit()
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    
  CSCTL0_H = 0xA5;
  CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting
  CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = VLO; MCLK = DCO
  CSCTL3 = DIVA_0 + DIVS_1 + DIVM_0;        // SMCLK = DCO/2 (4MHz) 
}

/** 
Configures hardware for the particular hardware platform:
- Ports: sets direction, interrupts, pullup/pulldown resistors etc. 
- Holds radio in reset (active-low)
*/
void halInit()
{
    //
    //    Initialize Oscillator
    //
    oscInit();
    
    //
    //    Initialize Ports
    //
    /*PORT 1:
    1.0     Module Reset [output]
    1.1     Module GPIO3 [n/c]
    1.2     Module MRDY  [output]
    1.3     Module Chip Select (also UCB0STE) [output]
    1.4     NTC Thermistor (100k) [Analog input A4]
    1.5     
    1.6     UCB0SIMO to Module
    1.7     UCB0SOMI to Module
    */
    
    P1DIR = BIT0 | BIT2 | BIT3;  // Configure outputs
    P1OUT &= ~BIT0;
    //RADIO_OFF();                  // Hold Module in Reset whist it is being configured  
    P1IE  = 0;       // interrupts
    P1IES = 0;       // Interrupt on high-to-low transition
    P1SEL1 = (BIT6 | BIT7);          // NOTE: set P1SEL1 and P1SEL0, BIT4 for thermistor analog input
    P1SEL0 = 0;
    P1IFG = 0;          // Clear P1 interrupts
    
    /*PORT 2
    2.0     Debug UART [UCA0TXD]
    2.1     Debug UART [UCA0RXD]
    2.2     UCB0CLK to Module
    2.3     module other
    2.4     SRDY [input]
    2.5     
    2.6     
    2.7     module other
    */
    
    P2DIR = 0;  // Configure outputs
    P2SEL1 = BIT0 | BIT1 | BIT2;
    P2SEL0 = 0;
    P2IE  = 0;       // interrupts
    P2IES = 0;       // Interrupt on high-to-low transition    
    P2IFG = 0;

    /*PORT 3
    3.0     Accelerometer X [analog A12]
    3.1     Accelerometer Y [analog A13]
    3.2     Accelerometer Z [analog A14]
    3.3     LDR (not populated)
    3.4     LED5
    3.5     LED6
    3.6     LED7
    3.7     LED8
    */
    P3DIR = BIT4 | BIT5 | BIT6 | BIT7;  // Configure outputs
    P3SEL1 = BIT0 | BIT1 | BIT2;
    P3SEL0 = BIT0 | BIT1 | BIT2;
    P3IE  = 0;       // interrupts
    P3IES = 0;       // Interrupt on high-to-low transition    
    P3IFG = 0;
    
    /*PORT 4
    4.0     CFG1 (tie high) - also button S1 - note: to prevent conflict, we set as input with pull-up
    4.1     module other - also button S2
    */    

    //P4DIR = BIT0;  // Configure outputs
    P4DIR = 0;  // Configure outputs    
    P4REN = BIT0;   // enable resistor
    P4SEL1 = 0;
    P4SEL0 = 0;
    P4IE  = 0;       // interrupts
    P4IES = 0;       // Interrupt on high-to-low transition    
    P4IFG = 0;
    P4OUT = BIT0;  //CONFIGURE AS PULL-UP RESISTOR
    
    /*PORT J
    J.0     LED1 / SMCLK
    J.1     LED2 / MCLK
    J.2     LED3 / ACLK
    J.3     LED4  
    J.4     XINR (not populated)
    J.5     XOUTR (not populated)
    */
    
    /* For Clock outputs */
    /* Note: PJ.0 (SMCLK) = TP10; PJ.1 (MCLK) = TP11; PJ.2 (ACLK) = TP12 */ 
    PJDIR = BIT0 | BIT1 | BIT2;  // Configure outputs
    PJSEL0 = BIT0 | BIT1 | BIT2; 
    PJSEL1 = 0; 
    
    /* For LEDs
    PJDIR = BIT0 | BIT1 | BIT2 | BIT3;  // Configure outputs
    PJSEL1 = 0;
    PJSEL2 = 0;    
    */
    
    
    //
    //    Initialize eUSCIA0 debug console:
    //
    // For computing register values, see 18.3.10 of MSP430FR57xx family guide
    // Step 1: 4,000,000 / 9,600 = 416.66
    // Decimal value (0.66) found in Table 18-4 -> UCBRSx = 0xD6      
    // Since (n = 416) is greater than 16, go to step 3.
    // Step 3: OS16 = 1; UCBRx = INT(n/16); UCBRF = INT([(N/16) - INT(N/16)])
    //For baud rate values, see Table18-5 in MSP430FR57xx family guide
    //For baud rate of 9600 with SMCLK of 4MHz:
    //UCOS16 = 1; UCBRx = 26; UCBRFx = 0; UCBRSx = 0xB6
  UCA0CTL1 |= UCSWRST; 
  UCA0CTL1 = UCSSEL_2;                      // Set SMCLK (4Mhz) as UCBRCLK
  UCA0BR0 = 26;                              // 9600 baud
  UCA0BR1 = 0; 
  UCA0MCTLW |= (0xD600 | UCOS16);           // Remainder of 4,000,000 / 9600 = 0.66 -> see Table 18-4
  UCA0CTL1 &= ~UCSWRST;                     // release from reset
  UCA0IE |= UCRXIE;                         // enable RX interrupt      

    //
    //   Deselect SPI peripherals:
    //
    SPI_SS_CLEAR();                       // Deselect Module
    
    //  Stop Timer A:
    //TACTL = MC_0; 
    
    //Point the function pointers to doNothing() so that they don't trigger a restart
    debugConsoleIsr = &doNothing;
    buttonIsr = &doNothing;

    halSpiInitModule();
}



/** 
Send one byte via hardware UART. Required for printf() etc. in stdio.h 
*/
int putchar(int c)
{	
    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = (uint8_t) (c & 0xFF); 
    return c;
}

/**
Initializes the Serial Peripheral Interface (SPI) interface to the Zigbee Module (ZM).
@note Maximum module SPI clock speed is 4MHz. SPI port configured for clock polarity of 0, clock phase of 0, and MSB first.
@note On the FRAM board the MSP430 uses USCIB0 SPI port to communicate with the module.
@pre SPI pins configured correctly: 
- Clock, MOSI, MISO configured as SPI function
- Chip Select configured as an output
- SRDY configured as an input.
@post SPI port is configured for communications with the module.
*/

void halSpiInitModule()
{
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 |= UCCKPH | UCMSB | UCMST | UCSYNC;         // 3-pin, 8-bit SPI slave
    UCB0CTLW0 |= UCSSEL_2;                    // ACLK
    UCB0BR0 = 2;  UCB0BR1 = 0;                      //SPI running at 2MHz (SMCLK / 2)
    UCB0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
}

/**
Sends a message over SPI to the Module.
The Module uses a "write-to-read" approach: to read data out, you must write data in.
This is a private method that gets wrapped by other methods, e.g. spiSreq(), spiAreq, etc.
To Write, set *bytes and numBytes. To Read, set *bytes only. Don't need to set numBytes because the 
Module will stop when no more bytes are received.
@param bytes the data to be sent or received.
@param numBytes the number of bytes to be sent. This same buffer will be overwritten with the received data.
@pre SPI port configured for the Module and Module has been initialized properly
@post bytes contains received data, if any
*/
void spiWrite(uint8_t *bytes, uint8_t numBytes)
{
    while (numBytes--)
    {  
        UCB0TXBUF = *bytes;
        while (!(UCB0IFG & UCRXIFG));
        *bytes++ = UCB0RXBUF;             //read bytes
    }
}

/** 
A fairly accurate blocking delay for waits in the millisecond range. Good for 1mSec to 1000mSec. 
At 1MHz, error of zero for 100mSec or 1000mSec. For 10mSec, error of 100uSec. At 1mSec, error is 20uSec.
At 8MHz, error of zero for 1000mSec.
Note that accuracy will depend on the clock source. MSP430F2xx internal DCO is typically +/-1%. 
For better timing accuracy, use a timer, or a crystal.
@pre TICKS_PER_MS is defined correctly
@param delay number of milliseconds to delay
*/
void delayMs(uint16_t delay)
{
    while (delay--)
    {
        __delay_cycles(TICKS_PER_MS);
    }
}

/** 
Turns ON the specified LED. 
@param led the LED to turn on
@post The specified LED is turned on. 
@return 0 if success, -1 if invalid LED specified
*/
int16_t setLed(uint8_t led)
{
    switch (led)
    {
    case 0:
        P3OUT |= BIT4;     // Turn on LED0
        return 0;
    case 1:
        P3OUT |= BIT5; 
        return 0;
    case 2:
        P3OUT |= BIT6;     // Turn on LED0
        return 0;
    case 3:
        P3OUT |= BIT7; 
        return 0;        
    default:
        return -1;
    }
}

/** 
Turns OFF LEDs. 
@post LEDs are turned off. 
*/
void clearLeds()
{
    P3OUT &= ~(BIT4|BIT5|BIT6|BIT7);
}

/** 
Toggles the specified LED. 
@param led the LED to toggle.
@post The specified LED is toggled. 
@return 0 if success, -1 if invalid LED specified
*/
int16_t toggleLed(uint8_t led)
{
    switch (led)
    {
    case 0:
        P3OUT ^= BIT4;     // Turn on LED0
        return 0;
    case 1:
        P3OUT ^= BIT5; 
        return 0;
    case 2:
        P3OUT ^= BIT6;     // Turn on LED0
        return 0;
    case 3:
        P3OUT ^= BIT7; 
        return 0;        
    default:
        return -1;
    }
}
