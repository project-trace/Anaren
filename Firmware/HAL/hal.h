/**
 * @ingroup hal
 * @{
 * @file hal.h
 *
 * @brief Hardware Abstraction Layer (HAL) file
 *
 * This library is designed to make it easy to use the same code across multiple hardware platforms.
 * This file allows you to use project properties to select which HAL file gets included.
 *
 * @note If changing hardware platforms add your file in the list below. You must also set the board
 * version (e.g. MY_SUPER_DUPER_PROTOTYPE_BOARD) in Project Properties. Also be sure to add your hal
 * file to the IAR project.
 * @see hal_helper.c for utilities to assist when changing hardware platforms
*
* $Rev: 1642 $
* $Author: dsmith $
* $Date: 2012-10-31 11:17:04 -0700 (Wed, 31 Oct 2012) $
*
* @section support Support
* Please refer to the wiki at http://teslacontrols.com/mw/ for more information. Additional support
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

#ifdef MDB1                       //Module Development Board 1, with MSP430F248
    //#error "MDB1 is not supported in this release"
    #include "../HAL/hal_mdb1.h"
    #include "../HAL/hal_msp430x2xxx_oscillators.h"
    #include "../HAL/hal_msp430x2xxx_usci.h"
    #include "../HAL/hal_msp430F2xxx_i2c.h"
    //#include "hal_msp430F2xxx.h"
#elif defined GW0
    #error "GW0 is not supported in this release"
    #include "../HAL/hal_gw0.h"	  //Zigbee to Ethernet Gateway GW0
    #elif defined GW1
    #include "../HAL/hal_gw1.h"	  //Zigbee to Ethernet Gateway GW1
    #ifdef HAL_I2C
        #include "../HAL/hal_stellaris_i2c.h"
    #endif
#elif defined EK_LM4F120XL
    #include "../HAL/hal_ek-lm4f120xl.h"	  //Stellaris LaunchPad
    //#include "../HAL/hal_stellaris_i2c.h"
    //#include "../HAL/hal_stellaris_bit_bang_i2c.h"
    #include "../HAL/hal_stellaris_softi2c.h"
#elif defined MDB2
    //#error "MDB2 is not supported in this release"
    #include "../HAL/hal_mdb2.h"		
    #include "../Common/printf.h"
#elif defined LAUNCHPAD
    #include "../HAL/hal_launchpad.h"
    #include "../HAL/hal_bit_bang_i2c.h"
#elif defined FRAM
    #include "../HAL/hal_fram.h"
    #include "../Common/printf.h"
#else
    #error "You must define a board configuration: MDB1, MDB2, LAUNCHPAD, or GW0. In IAR this is done in Project Options : C/C++ Compiler : Preprocessor : Defined Symbols. In CCS this is done in Project Properties : Build : MSP430 Compiler : Advanced Options : Predefined Symbols."
#endif

/* @} */
