/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_fragmentation_router_afzdo.c
 *
 * @brief Resets Module, configures this device to be a Zigbee Router, joins a network, then sends an
 * extended length message to the coordinator periodically.
 * Uses the AF/ZDO interface.
 * Pressing the button will display device information.
 *
 * This matches example_fragmentation_coordinator.c
 * Get the coordinator running first, or else the router won't have anything to join to.
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
#include "../ZM/application_configuration.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../Common/utilities.h"
#include "module_example_utils.h"
#include <stdint.h>

/** function pointer (in hal file) for the function that gets called when a button is pressed*/
extern void (*buttonIsr)(int8_t);

void handleButtonPress(int8_t);

#define RESTART_AFTER_ZM_FAILURE

moduleResult_t result;

/* Use a smaller message size if we're compiling for Launchpad. Launchpad cannot handle a full 600B 
 message because the MSP430F2553 only has 512B of RAM. The other platforms (Stellaris, MDB1, etc.)
 use processors with more RAM and can support a longer message. */
#ifdef LAUNCHPAD
#define MESSAGE_LENGTH     130
#else
#define MESSAGE_LENGTH     AF_DATA_REQUEST_EXT_MAX_TOTAL_PAYLOAD_LENGTH
#endif

/* Buffer to contain the message that we wish to send */
uint8_t testMessage[MESSAGE_LENGTH];    // Note: this causes a faultISR if declared in main application body

int main( void )
{
    halInit();
    moduleInit();    
    printf("\r\n****************************************************\r\n");    
    printf("Fragmentation Example - ROUTER - using AFZDO\r\n");
    buttonIsr = &handleButtonPress;    
    HAL_ENABLE_INTERRUPTS();

#define MODULE_START_DELAY_IF_FAIL_MS 5000

    /* See basic communications examples for more information about module startup. */
    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_ROUTER;
    start: 
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS);
    }
    printf("On Network!\r\n");
    setLed(0);
    /* On network, display info about this network */ 
#ifdef DISPLAY_NETWORK_INFORMATION    //excluded to reduce code size
    displayNetworkConfigurationParameters();   
    displayDeviceInformation();
#endif 

    /* Now the network is running - send a message to the coordinator every few seconds.*/
#define TEST_CLUSTER 0x77    

    uint8_t counter = 0;
    while (1)
    {       
        /* Fill test message buffer with DEADBEEF */
        initializeBuffer(testMessage, MESSAGE_LENGTH);

        printf("Sending Message #%u L%u to Short Address 0x0000 (Coordinator) ", counter++, MESSAGE_LENGTH);
        /* Send an extended length message to a short address */
        moduleResult_t result = afSendDataExtendedShort(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, 0, TEST_CLUSTER, testMessage, MESSAGE_LENGTH);  //a short message - coordinator will receive an AF_INCOMING_MSG_EXT
        if (result == MODULE_SUCCESS)
        {
            printf("Success\r\n");
        } else {
            printf("ERROR %i ", result);
#ifdef RESTART_AFTER_ZM_FAILURE
            printf("\r\nRestarting\r\n");
            goto start;
#else        
            printf("stopping\r\n");
            while(1);
#endif
        }
        delayMs(2000);  
    }   
}

/** When a button is pressed, display device information */
void handleButtonPress(int8_t btn)
{
    printf("Device Information:\r\n"); 
    displayDeviceInformation();               // if button still pressed then display device information
}

/* @} */
