/**
* @file hal_msp430x2xxx_oscillators.c
*
* @brief Hardware Abstraction Layer (HAL) toolbox of utilities for configuring 
* the oscillators in the MSP430x2xxx processors. 
* For example, used with the MSP430F248, or MSP430F2274, or MSP430G2553.
*
* This file contains generic utilities for using the oscillators on these processors.
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

#include "hal_msp430x2xxx_oscillators.h"

/** See delayMs() in main hal file. Included again here to avoid circular dependencies. */
void dly(uint16_t delay)
{
    while (delay--)
    {
        __delay_cycles(TICKS_PER_MS);
    }
}

/** Calibrate VLO. Once this is done, the VLO can be used semi-accurately for timers etc. 
Once calibrated, VLO is within ~2% of actual when using a 1% calibrated DCO frequency and temperature and supply voltage remain unchanged.
@return VLO frequency (number of VLO counts in 1sec), or -1 if out of range
@pre SMCLK is 4MHz
@pre MCLK is 8MHz
@pre ACLK sourced by VLO (BCSCTL3 = LFXT1S_2 in MSP430F2xxx)
@note Calibration is only as good as MCLK source. Obviously, if using the internal DCO (+/- 1%) then this value will only be as good as +/- 1%. YMMV.
@note On MSP430F248 or MSP430F22x2 or MSP430F22x4, must use TACCR2. On MSP430F20x2, must use TACCR0. On MSP430G2553, must use CCI0B on CCR0
Check device-specific datasheet to see which module block has ACLK as a compare input.
Modify TACCTLx, TACCRx, and CCIS_x accordingly
For example, see page 23 of the MSP430F24x datasheet or page 17 of the MSP430F20x2 datasheet, or page 18 of the MSP430F22x4 datasheet.
@note If application will require accuracy over change in temperature or supply voltage, recommend calibrating VLO more often.
@post Timer A settings restored to what they were beforehand except for TACCR0 which is reset.
*/

int16_t halCalibrateVlo()
{
    WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
    dly(1000);                        // Wait for oscillators to settle
    uint16_t temp_BCSCTL1 = BCSCTL1;
    uint16_t temp_TACCTL0 = TACCTL0;
    uint16_t temp_TACTL = TACTL;
    
    BCSCTL1 |= DIVA_3;                    // Divide ACLK by 8
    TACCTL0 = CM_1 + CCIS_1 + CAP;        // Capture on ACLK
    TACTL = TASSEL_2 + MC_2 + TACLR;      // Start TA, SMCLK(DCO), Continuous
    while ((TACCTL0 & CCIFG) == 0);       // Wait until capture
    
    TACCR0 = 0;                           // Ignore first capture
    TACCTL0 &= ~CCIFG;                    // Clear CCIFG
    
    while ((TACCTL0 & CCIFG) == 0);       // Wait for next capture
    unsigned int firstCapture = TACCR0;   // Save first capture
    TACCTL0 &= ~CCIFG;                    // Clear CCIFG
    
    while ((TACCTL0 & CCIFG) ==0);        // Wait for next capture
    
    unsigned long counts = (TACCR0 - firstCapture);        // # of VLO clocks in 8Mhz
    BCSCTL1 = temp_BCSCTL1;                  // Restore original settings
    TACCTL0 = temp_TACCTL0;
    TACTL = temp_TACTL;
    
    //TACCTL0 = 0; TACTL = 0;                 // Clear Timer settings

    signed int aClk = ((unsigned int) (32000000l / counts));
    if ((aClk > VLO_MIN) && (aClk < VLO_MAX))
        return aClk;
    else
        return -1;    
    
}

/** Measure ACLK clock source frequency using main clock. Ignores ACLK Divide settings (DIVA bits in BCSCTL1)
@return ACLK frequency (number of ACLK counts in 1sec)
@pre SMCLK is 4MHz
@pre MCLK is 8MHz
@note on MSP430F248 or MSP430F22x2 or MSP430F22x4, must use TACCR2. On MSP430F20x2, must use TACCR0.
Check device-specific datasheet to see which module block has ACLK as a compare input.
For example, see page 23 of the MSP430F24x datasheet or page 17 of the MSP430F20x2 datasheet, or page 18 of the MSP430F22x4 datasheet.
@post Timer A settings changed
@post WDT disabled
@post ACLK divide by 8 bit cleared
*/
uint16_t halMeasureAclkFrequency()
{
    WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer

    uint8_t aclkDivisor = (BCSCTL1 & DIVA_3);                   // Store ACLK Divisor settings
    
    BCSCTL1 |= DIVA_3;                    // Divide ACLK by 8
    TACCTL2 = CM_1 + CCIS_1 + CAP;        // Capture on ACLK
    TACTL = TASSEL_2 + MC_2 + TACLR;      // Start TA, SMCLK(DCO), Continuous
    while ((TACCTL2 & CCIFG) == 0);       // Wait until capture
    
    TACCR2 = 0;                           // Ignore first capture
    TACCTL2 &= ~CCIFG;                    // Clear CCIFG
    
    while ((TACCTL2 & CCIFG) == 0);       // Wait for next capture
    uint16_t firstCapture = TACCR2;   // Save first capture
    TACCTL2 &= ~CCIFG;                    // Clear CCIFG
    
    while ((TACCTL2 & CCIFG) ==0);        // Wait for next capture
    
    uint32_t counts = (TACCR2 - firstCapture);        // # of SMCLK clocks in one ACLK pulse

    BCSCTL1 &= ~DIVA_3;                   // Restore ACLK Divisor settings
    BCSCTL1 |= aclkDivisor;
    
    uint16_t aClk = ((uint16_t) (32000000l / counts));
    return aClk;
}