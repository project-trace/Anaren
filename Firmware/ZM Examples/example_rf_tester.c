/**
 * @ingroup moduleInterface
 * @{
 *
 * @file example_rf_tester.c
 *
 * @brief Simple utility to configure the Module to output an RF test signal.
 *
 * Configures Module interface, resets module, and then processes typed commands.
 * The Module can output either an RF carrier signal, or random modulated data.
 * The user can change which test mode to use, channel (11-26), and turn off the transmitter.
 * This utility can be used for radiated emissions compliance testing or tuning the RF circuit.
 *
 * The Utility can be controlled through the simple command-line menu. Commands:
 *     ]   Increment RF Test Channel
 *     [   Increment RF Test Channel
 *     M   Change RF Test Mode to modulated output
 *     U   Change RF Test Mode to UNmodulated output
 *     R   Change RF Test Mode to receive (Turn off output)
 *     !   Reset Module
 *     ?   Print this menu
 *
 * If, instead, you would like it to output one particular mode on startup, change
 * <code>rfTestMode</code> and <code>rfTestChannel</code> accordingly.
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

#include "../HAL/hal.h"
#include "../ZM/module.h"
#include "../ZM/module_errors.h"
#include "../ZM/zm_phy_spi.h"
#include <stdint.h>

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

#define NO_CHARACTER_RECEIVED 0xFF

/** Which RF Test Mode we're in. Will be changed by the commands. 
Change this if you want the application to start in a different mode. */
uint8_t rfTestMode = RF_TEST_UNMODULATED;

/** The current channel that the RF test mode is set to. 
Stored so that we can increment/decrement this channel easily. 
Change this if you want the application to start on a different channel. */
uint8_t rfTestChannel = 11;

/** The RF Output Power used for testing. Note that this is the value output by the CC2530 itself.
If the module has a PA/LNA (CC2590, CC2591, etc.) then the actual transmitted output power will be
higher of course. See module.h for valid values.*/
#define RF_OUTPUT_POWER         RF_OUTPUT_POWER_PLUS_4_5_DBM

/** the command that was entered by the user */
volatile char command = NO_CHARACTER_RECEIVED;

/** function pointer (in hal file) for the function that gets called when a byte is received on the 
debug console. */
extern void (*debugConsoleIsr)(int8_t);  

/** locally function to process bytes received on the debug console */
void handleDebugConsoleInterrupt(int8_t rxByte);

void printUsage()
{
    printf("Commands:\r\n");    
    printf("    ]   Increment RF Test Channel\r\n");
    printf("    [   Increment RF Test Channel\r\n");
    printf("    M   Change RF Test Mode to modulated output\r\n");
    printf("    U   Change RF Test Mode to UNmodulated output\r\n");    
    printf("    R   Change RF Test Mode to receive (Turn off output)\r\n");
    printf("    !   Reset Module\r\n");   
    printf("    ?   Print this menu\r\n");    
}

void setTestMode()
{    
    uint16_t frequency = 2405 + 5*(rfTestChannel-11);  //CC253x family datasheet, equation 19-1
    printf("Mode = %s, chan. %u (frequency = %uMHz): ", getRfTestModeName(rfTestMode), rfTestChannel, frequency);
    moduleReset();
    if (setRfTestMode(rfTestMode, rfTestChannel, RF_OUTPUT_POWER, 0) != MODULE_SUCCESS)
        printf("ERROR\r\n");
    else
        printf("Success - Applying..\r\n");
    //RESET the Module to apply these settings
    RADIO_OFF();
    delayMs(1);
    RADIO_ON();
    printf("Ready for new command:\r\n");
}

int main( void )
{
    halInit();
    moduleInit();    
    printf("\r\n************************************************\r\nRF Tester\r\n");
    printf("\r\nResetting Radio, then starting RF test output\r\n");    
    if (moduleReset() == MODULE_SUCCESS)
    {
        displaySysResetInd();  // Display the contents of the received SYS_RESET_IND message
    } else {
        printf("ERROR\r\n");
    }
    printUsage();
    debugConsoleIsr = &handleDebugConsoleInterrupt;   //call method handleDebugConsoleInterrupt() when a byte is received
    setTestMode();  //Turn on RF Test Mode using initial conditions
    HAL_ENABLE_INTERRUPTS();    

    /* loop continually, processing any commands typed on the command line */
    while (1) 
    {
        if (command != NO_CHARACTER_RECEIVED)
        {
            printf("\r\n", command);
            switch (command)    //based on what command was entered, set the RF test mode and/or channel accordingly
            {
            case ']':
                printf("Increment RF Test Channel\r\n");
                if (rfTestChannel == CHANNEL_MAX)
                {
                    printf("Error - already at the highest channel, %u\r\n", CHANNEL_MAX);
                    command = NO_CHARACTER_RECEIVED;
                    continue;
                }
                rfTestChannel++;
                break;        
            case '[':
                printf("Decrement RF Test Channel\r\n");
                if (rfTestChannel == CHANNEL_MIN)
                {
                    printf("Error - already at the lowest channel, %u\r\n", CHANNEL_MIN);
                    command = NO_CHARACTER_RECEIVED;
                    continue;
                }
                rfTestChannel--;         
                break;
            case 'M':
            case 'm':
                printf("Changing RF Test Mode to modulated output\r\n");
                rfTestMode = RF_TEST_MODULATED;
                break;
            case 'U':
            case 'u':
                printf("Changing RF Test Mode to UNmodulated output\r\n");
                rfTestMode = RF_TEST_UNMODULATED;
                break;    
            case 'R':
            case 'r':
                printf("Changing RF Test Mode to receive\r\n");
                rfTestMode = RF_TEST_RECEIVE;
                break;  
            case '!':
                printf("Resetting Module\r\n");                
                moduleReset();
                command = NO_CHARACTER_RECEIVED;
                continue;   
            case '?':
                printUsage();
                command = NO_CHARACTER_RECEIVED;
                continue;
            default:
                printf("Unknown Command\r\n");
                command = NO_CHARACTER_RECEIVED;
                continue;  //return; don't change anything
            }

            setTestMode();            //now, set the RF test mode and channel based on what was entered
            command = NO_CHARACTER_RECEIVED;
        }
    }
}

/** Method to handle bytes received on the debug console. Just stores it to the variable 
<code>command</code>. This gets called by the ISR in the hal file since we set the debugConsoleIsr 
function pointer (in hal file) to point to this function.
 */
void handleDebugConsoleInterrupt(int8_t rxByte)
{
    command = rxByte;
}



/* @} */
