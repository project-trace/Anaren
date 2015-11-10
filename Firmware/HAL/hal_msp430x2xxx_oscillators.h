/**
*  @file hal_msp430x2xxx_oscillators.h
*
*  @brief  public methods and macros for hal_msp430x2xxx_oscillators.c
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

#ifndef HAL_MSP430X2XXX_OSCILLATORS_H
#define HAL_MSP430X2XXX_OSCILLATORS_H

#ifdef MDB1
#include "msp430x24x.h"
#elif defined MDB2
#error "MDB2 is not supported in this release"
#include "msp430x22x4.h"
#elif defined LAUNCHPAD
#include "msp430g2553.h"
#else
#error "To use the oscillator library You must define a board configuration: MDB1, MDB2, or LAUNCHPAD. In IAR this is done in Project Options : C/C++ Compiler : Preprocessor : Defined Symbols. In CCS this is done in Project Properties : Build : MSP430 Compiler : Advanced Options : Predefined Symbols."
#endif
#include <stdint.h>

//VLO Utilities
int16_t halCalibrateVlo();
uint16_t halMeasureAclkFrequency();

#define XTAL 8000000L   //Clock speed - used below.
#define TICKS_PER_MS (XTAL / 1000)

#define HAL_DELAY_MS(halMsec) for (int i = halMsec; i; i--) for (int j = TICKS_PER_MS; j; j--) //delay 1mSec

#define HAL_DISABLE_WDT()           (WDTCTL = WDTPW + WDTHOLD)
#define HAL_WDT_ENABLE_ACLK_32K()   (WDTCTL = WDTPW+WDTCNTCL+WDTSSEL)
#define HAL_WDT_RESET              HAL_WDT_ENABLE_ACLK_32K

//Required for using VLO
#define VLO_NOMINAL 12000
#define VLO_MIN (VLO_NOMINAL - (VLO_NOMINAL >> 2)) //VLO min max = +/- 25% of nominal
#define VLO_MAX (VLO_NOMINAL + (VLO_NOMINAL >> 2))

//
//  Main Oscillator configuration macros - these configure MCLK and SMCLK
//

/** Main clock of 1MHz by DCO, SMCLK of 1MHz. Note: SMCLK of 1MHz is incompatible with many other library functions, like hal_flash_write. */
#define HAL_OSCILLATOR_CONFIGURE_MAIN_DCO_1MHZ() \
  if (CALBC1_1MHZ ==0xFF || CALBC1_1MHZ == 0xFF) \
    while(1);  \
      BCSCTL1 = CALBC1_1MHZ; DCOCTL = CALDCO_1MHZ; BCSCTL2 = 0   

/** Main clock of 8MHz by DCO, SMCLK of 4MHz */
#define HAL_OSCILLATOR_CONFIGURE_MAIN_DCO_8MHZ() \
  if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF) \
    while(1);  \
      BCSCTL1 = CALBC1_8MHZ; DCOCTL = CALDCO_8MHZ; BCSCTL2 = DIVS_1   

/** Main clock of 8MHz by XTAL, SMCLK of 4MHz. Requires an external 8MHz xtal. */
#define HAL_OSCILLATOR_CONFIGURE_MAIN_XTAL_8MHZ()   \
  BCSCTL1 &= ~XT2OFF;   BCSCTL3 |= XT2S_2;  \
    do { IFG1 &= ~OFIFG; for (unsigned int i = 0xFFFF; i > 0; i--); } \
      while (IFG1 & OFIFG);         \
        BCSCTL2 = (SELM_2 + DIVM_1 + SELS + DIVS_1)

/** Feel free to add additional oscillator configuration #defines here */

//
//  Auxilliary Oscillator configuration macros - these configure ACLK only
//

/** Configure ACLK Oscilator to use VLO. Note: you must calibrate VLO! */
#define HAL_OSCILLATOR_CONFIGURE_AUX_VLO()          BCSCTL3 = LFXT1S_2

/** Configure ACLK Oscilator to use an external 32kHz XTAL. Requires an external 32kHz xtal.*/
#define HAL_OSCILLATOR_CONFIGURE_AUX_XTAL_32KHZ()   BCSCTL3 = (LFXT1S_0 + XCAP_3)

/** Configure ACLK Oscilator to use an external 32kHz oscillator input. Requires an external 32kHz oscillator.*/
#define HAL_OSCILLATOR_CONFIGURE_AUX_EXT_32KHZ()    BCSCTL3 = LFXT1S_3

/** Configure ACLK to be one fourth of the ACLK clock source */
#define HAL_OSCILLATOR_CONFIGURE_AUX_DIVIDE_BY_4()  BCSCTL1 |= DIVA_2  



#define HAL_SYSTICK_INTERVAL_MS 250

#endif
