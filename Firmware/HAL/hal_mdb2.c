/**
* @file hal_mdb2.c
*
* @brief Hardware Abstraction Layer (HAL) for the MDB2
*
* This file must be modified if changing hardware platforms.
*
* The Zigbee Module library & examples require several methods to be defined. 
* See hal_helper documentation. Also see hal_mdb2.h for macros that must be defined.
*
* @see hal_helper.c for utilities to assist when changing hardware platforms
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

#include "hal_mdb2.h"
#include <stdint.h>

/** This is a function pointer for the Interrupt Service Routine called when a debug console character is received.
To use it, declare it with
<code> extern void (*debugConsoleIsr)(int8_t);  </code> 
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

/** The post-calibrated frequency of the Very Low Oscillator (VLO). This MUST be 
calibrated by calibrateVlo() prior to use. Set with calibrateVlo() and read by 
initTimer(). */
uint16_t vloFrequency = 0;

/** Debug console interrupt service routine, called when a byte is received on USCIA0. */
#pragma vector = USCIAB0RX_VECTOR 
__interrupt void USCIAB0RX_ISR(void)
{
    if (IFG2 & UCA0RXIFG)               //debug console character received
    {
        debugConsoleIsr(UCA0RXBUF);    //reading this register clears the interrupt flag
    } 
}

/** Port 1 interrupt service routine, called when an interrupt-enabled pin on port 1 changes state. */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    if (P1IFG & BIT2)    
    {
        buttonIsr(0);   // Button 0 was pressed
        if (wakeupFlags & WAKEUP_AFTER_BUTTON)    
            HAL_WAKEUP();   
    }
    
    P1IFG = 0;                          // clear the interrupt
}

/** Port 2 interrupt service routine, called when an interrupt-enabled pin on port 2 changes state. */
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    if (P2IFG & BIT6)
    {
        srdyIsr();
        if (wakeupFlags & WAKEUP_AFTER_SRDY)    
            HAL_WAKEUP();          
    }
    
    P2IFG = 0;                          // clear the interrupt
}

/** Simple placeholder to point the function pointers to so that they don't cause mischief */
void doNothing(int8_t a)
{
    __delay_cycles(1);  //prevent this from getting optimized out
}

/** Initializes Oscillator: turns off WDT, configures MCLK = 8MHz using internal DCO & sets SMCLK = 4MHz */
void oscInit()
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    
    if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF)                                     
    {  
        while(1); // Stop if calibration constants erased
    }   
    BCSCTL1 = CALBC1_8MHZ;          // Set DCO = 8MHz for MCLK
    DCOCTL = CALDCO_8MHZ;
    BCSCTL2 |= DIVS_1;              // SMCLK = DCO/2 (4MHz) 
    BCSCTL3 |= LFXT1S_2;            // Use VLO for ACLK
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
    1.0     LED 1
    1.1     LED 2
    1.2     Button - connects to GND when closed
    1.3     
    1.4     
    1.5     
    1.6     
    1.7     
    */
    P1DIR = BIT0+BIT1;  // Configure LED pins as outputs
    P1OUT &= ~BIT2;     
    P1IE  = BIT2;       // Enable Button Interrupt
    P1IES = BIT2;       // Interrupt on high-to-low transition
    P1REN = BIT2;       // Enable resistor for button
    P1OUT = BIT2;       // make resistor a PULL-UP
    P1SEL = 0;          // NOTE: default value is NOT 0!
    P1IFG = 0;          // Clear P1 interrupts
    
    /*PORT 2
    2.0     
    2.1     
    2.2     TEST
    2.3     
    2.4     
    2.5     
    2.6     SRDY
    2.7     
    */
    P2DIR = BIT2;  // Configure Test signal as output
    P2SEL = 0;
    
    /*PORT 3
    3.0     ZM CS
    3.1     SPI MOSI
    3.2     SPI MISO
    3.3     SPI SCLK
    3.4     Debug UART Tx
    3.5     Debug UART Rx
    3.6     MRDY
    3.7     ZM Reset
    */
    P3DIR = BIT0+BIT6+BIT7;
    P3SEL = (BIT1+BIT2+BIT3+BIT4+BIT5);  //p3.1,2,3=USCI_B0; P3.4,5 = USCI_A0 TXD/RXD;
    P3OUT &= ~BIT7;                      //turn off radio
    
    /*PORT 4
    4.0     
    4.1     
    4.2     
    4.3     
    4.4     
    4.5     
    4.6     
    4.7     
    */
    P4DIR = BIT0+BIT1 + BIT4;  //BIT4 = DEBUGGING
    P4SEL = 0;
    P4OUT &= ~BIT4;
    
    //
    //    Initialize UART debug console:
    //
    UCA0CTL1 |= UCSSEL_2;                     // USCIA0 source from SMCLK
    UCA0BR0 = 26; UCA0BR1 = 0;                // 4mHz smclk w/modulation for 9,600bps, table 15-5 
    UCA0MCTL = UCBRS_0 + +UCBRF_1 + UCOS16;   // Modulation UCBRSx=1, over sampling      
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
    
    //
    //   Deselect SPI peripherals:
    //
    SPI_SS_CLEAR();                       // Deselect Module
    
    //  Stop Timer A:
    TACTL = MC_0; 
    
    //Point the function pointers to doNothing() so that they don't trigger a restart
    debugConsoleIsr = &doNothing;
    buttonIsr = &doNothing;

    halSpiInitModule();    
    
    clearLeds();
}



/** Configures SRDY interrupt. Does NOT enable the interrupt.
SRDY is P2.6
@param flags the options for the SRDY interrupt: 
- SRDY_INTERRUPT_FALLING_EDGE or SRDY_INTERRUPT_RISING_EDGE
- NO_WAKEUP or WAKEUP_AFTER_SRDY

@return 0 if success, -1 if bad flag
*/
int16_t halConfigureSrdyInterrrupt(uint8_t flags)
{
    if (flags & (~(SRDY_INTERRUPT_FALLING_EDGE | WAKEUP_AFTER_SRDY)))   //if any other bit is set in flags
        return -1;                                                      //then error
    
    if (flags & SRDY_INTERRUPT_FALLING_EDGE)
        P2IES |= BIT6;      //Interrupt on high-to-low transition
    else
        P2IES &= ~BIT6;     //Interrupt on low-to-high transition        
    
    if (flags & WAKEUP_AFTER_SRDY)
        wakeupFlags |= WAKEUP_AFTER_SRDY;
    else
        wakeupFlags &= ~WAKEUP_AFTER_SRDY;
    return 0;
}

/** Send one byte via hardware UART. Required for printf() etc. in stdio.h */
int16_t putchar(int16_t c)
{	
    while (!(IFG2 & UCA0TXIFG));   // Wait for ready
    UCA0TXBUF = (uint8_t) (c & 0xFF); 
    return c;
}

/**
Initializes the Serial Peripheral Interface (SPI) interface to the Zigbee Module (ZM).
@note Maximum module SPI clock speed is 4MHz. SPI port configured for clock polarity of 0, clock phase of 0, and MSB first.
@note On the MDB2 the MSP430 uses USCIB0 SPI port to communicate with the module.
@pre SPI pins configured correctly: 
- Clock, MOSI, MISO configured as SPI function
- Chip Select configured as an output
- SRDY configured as an input.
@post SPI port is configured for communications with the module.
*/
void halSpiInitModule()
{
    UCB0CTL1 |= UCSSEL_2 | UCSWRST;                 //serial clock source = SMCLK, hold SPI interface in reset
    UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCSYNC;     //clock polarity = inactive is LOW (CPOL=0); Clock Phase = 0; MSB first; Master Mode; Synchronous Mode    
    UCB0BR0 = 2;  UCB0BR1 = 0;                      //SPI running at 2MHz (SMCLK / 2)
    UCB0CTL1 &= ~UCSWRST;                           //start USCI_B1 state machine  
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
        while (!(IFG2 & UCB0RXIFG)) ;     //WAIT for a character to be received, if any
        *bytes++ = UCB0RXBUF;             //read bytes
    }
}

/** A fairly accurate blocking delay for waits in the millisecond range. Good for 1mSec to 1000mSec. 
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

/** Turns ON the specified LED. 
@param led the LED to turn on, must be 0 or 1.
@post The specified LED is turned on. 
@return 0 if success, -1 if invalid LED specified
*/
int16_t setLed(uint8_t led)
{
    if (led > (NUMBER_OF_LEDS-1)) 
        return -1;
    P1OUT |= (led) ? BIT1 : BIT0;
    return 0;
}

/** Turns OFF LEDs. 
@post LEDs are turned off. 
*/
void clearLeds()
{
    P1OUT &= ~(BIT0 + BIT1);
}

/** Toggles the specified LED. 
@param led the LED to toggle, must be 0 or 1.
@post The specified LED is toggled. 
@return 0 if success, -1 if invalid LED specified
*/
int16_t toggleLed(uint8_t led)
{
    if (led > (NUMBER_OF_LEDS-1)) 
        return -1;
    P1OUT ^= (1 << led);
    return 0;
}

//
//
//          MDB2 PERIPHERALS
//          NOT REQUIRED FOR MODULE OPERATION
//
//


/** Reads the MSP430 supply voltage using the Analog to Digital Converter (ADC). 
On MDB2, this is approx. 3600mV
@return Vcc supply voltage, in millivolts
*/
uint16_t getVcc3()
{
    ADC10CTL0 = SREF_1 + REFON + REF2_5V + ADC10ON + ADC10SHT_3;  // use internal ref, turn on 2.5V ref, set samp time = 64 cycles
    ADC10CTL1 = INCH_11;                         
    delayMs(1);                                     // Allow internal reference to stabilize
    ADC10CTL0 |= ENC + ADC10SC;                     // Enable conversions
    while (!(ADC10CTL0 & ADC10IFG));                // Conversion done?
    unsigned long temp = (ADC10MEM * 5000l);        // Convert raw ADC value to millivolts
    return ((uint16_t) (temp / 1024l));
}

/** Configures all module interface signals as inputs to allow the module to be programmed.
Toggles LED0 quickly to indicate application is running. 
*/
void halSetAllPinsToInputs(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF)                                     
    {  
        while(1); // Stop if calibration constants erased
    }   
    BCSCTL1 = CALBC1_8MHZ; // Set DCO = 8MHz for MCLK
    DCOCTL = CALDCO_8MHZ;
    BCSCTL2 |= DIVS_1;     //SMCLK = DCO/2 (4MHz)    
    
    P1DIR = 1; P1REN = 0; P1IE = 0; P1SEL = 0; 
    P2DIR = 0; P2REN = 0; P2IE = 0; P2SEL = 0; 
    P3DIR = 0;
    P4DIR = 0; P4REN = 0; P4SEL = 0;   
    for (;;)
    {
        toggleLed(0);
        delayMs(100);   
    }
}

#define TIMER_MAX_SECONDS 4
/** Configures timer.
@pre ACLK sourced from VLO
@pre VLO has been calibrated; number of VLO counts in one second is in vloFrequency.
@param seconds period of the timer. Maximum is 0xFFFF / vloFrequency; or about 4 since VLO varies between 9kHz - 15kHz. 
Use a prescaler on timer (e.g. set IDx bits in TACTL register) for longer times. 
Maximum prescaling of Timer A is divide by 8. Even longer times can be obtained by prescaling ACLK if this doesn't affect other system peripherals.
@return 0 if success; -1 if illegal parameter or -2 if VLO not calibrated
*/
int16_t initTimer(uint8_t seconds)
{  
    if (seconds > TIMER_MAX_SECONDS)
        return -1;
    if (vloFrequency == 0)
        return -2;
    if ((wakeOnTimer != NO_WAKEUP) && (wakeOnTimer != WAKEUP_AFTER_TIMER))
        return -3;
    wakeupFlags |= WAKEUP_AFTER_TIMER;
    CCTL0 = CCIE;                             // CCR0 interrupt enabled
    CCR0 = vloFrequency * (seconds);                      // generate int
    TACTL = TASSEL_1 + MC_1;           // ACLK, upmode
    return 0;
}

void stopTimer()
{
    TACTL = MC_0; 
}

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    timerIsr();
    if (wakeupFlags & WAKEUP_AFTER_TIMER)    
    {
        HAL_WAKEUP();     
    }
}

/** Calibrate VLO. Once this is done, the VLO can be used semi-accurately for timers etc. 
Once calibrated, VLO is within ~2% of actual when using a 1% calibrated DCO frequency and temperature and supply voltage remain unchanged.
@return VLO frequency (number of VLO counts in 1sec), or -1 if out of range
@pre SMCLK is 4MHz
@pre MCLK is 8MHz
@pre ACLK sourced by VLO (BCSCTL3 = LFXT1S_2; in MSP430F2xxx)
@note Calibration is only as good as MCLK source. Obviously, if using the internal DCO (+/- 1%) then this value will only be as good as +/- 1%. YMMV.
@note On MSP430F248 or MSP430F22x2 or MSP430F22x4, must use TACCR2. On MSP430F20x2, must use TACCR0.
Check device-specific datasheet to see which module block has ACLK as a compare input.
For example, see page 23 of the MSP430F24x datasheet or page 17 of the MSP430F20x2 datasheet, or page 18 of the MSP430F22x4 datasheet.
@note If application will require accuracy over change in temperature or supply voltage, recommend calibrating VLO more often.
@post Timer A settings changed
@post ACLK divide by 8 bit cleared
*/

int16_t calibrateVlo()
{
    WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
    delayMs(1000);                        // Wait for oscillators to settle
    
    BCSCTL1 |= DIVA_3;                    // Divide ACLK by 8
    TACCTL2 = CM_1 + CCIS_1 + CAP;        // Capture on ACLK
    TACTL = TASSEL_2 + MC_2 + TACLR;      // Start TA, SMCLK(DCO), Continuous
    while ((TACCTL0 & CCIFG) == 0);       // Wait until capture
    
    TACCR2 = 0;                           // Ignore first capture
    TACCTL2 &= ~CCIFG;                    // Clear CCIFG
    
    while ((TACCTL2 & CCIFG) == 0);       // Wait for next capture
    uint16_t firstCapture = TACCR2;   // Save first capture
    TACCTL2 &= ~CCIFG;                    // Clear CCIFG
    
    while ((TACCTL2 & CCIFG) ==0);        // Wait for next capture
    
    unsigned long counts = (TACCR2 - firstCapture);        // # of VLO clocks in 8Mhz
    BCSCTL1 &= ~DIVA_3;                   // Clear ACLK/8 settings
    
    vloFrequency = ((uint16_t) (32000000l / counts));
    if ((vloFrequency > VLO_MIN) && (vloFrequency < VLO_MAX))
        return vloFrequency;
    else
        return -1;
}
