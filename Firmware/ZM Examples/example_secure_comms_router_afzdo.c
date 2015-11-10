/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_secure_comms_router_afzdo.c
 *
 * @brief Resets Radio, configures this device to be a Zigbee Router, joins a network, initializes
 * security and then sends a message to the coordinator once per second.
 * Uses the AF/ZDO interface.
 *
 * This matches example_secure_communications_COORDINATOR.c
 * Get the coordinator running first, or else the router won't have anything to join to.
 *
 * Note: security mode should be the same for all devices on the network and can be one of:
- SECURITY_MODE_PRECONFIGURED_KEYS
- USE_SECURITY_MODE_COORD_DIST_KEYS
- SECURITY_MODE_OFF
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
#include "../ZM/module_utilities.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "module_example_utils.h"   //for handleReturnValue() and polling()

/** Used to store return value from module operations */
moduleResult_t result;     

#define RESTART_AFTER_ZM_FAILURE

/** encryption key used in security */
uint8_t key[16] = "*Tesla Controls*";

unsigned char counter = 0;
unsigned char testMessage[] = {0xE0,0xE1,0xE2,0xE3,0xE4};

int main( void )
{
    halInit();
    moduleInit();    
    printf("\r\n****************************************************\r\n");    
    printf("Secure Communications Example - ROUTER - using AFZDO\r\n");
    HAL_ENABLE_INTERRUPTS();

#define MODULE_START_DELAY_IF_FAIL_MS 5000

    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_ROUTER;
    defaultConfiguration.securityMode = SECURITY_MODE_PRECONFIGURED_KEYS;
    defaultConfiguration.securityKey = key;
    start:
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS);
    }

    printf("On Network!\r\n");
    setLed(0);

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();
    displayDeviceInformation();
#endif  

    /* Now the network is running - send a message to the coordinator every few seconds.*/
#define TEST_CLUSTER 0x77    

    while (1)
    {
        printf("Sending Message %u  ", counter++);
        result = afSendData(DEFAULT_ENDPOINT,DEFAULT_ENDPOINT,0, TEST_CLUSTER, testMessage, 5);
        if (result == MODULE_SUCCESS)
        {
            printf("Success\r\n");
        } else {
            printf("ERROR %02X ", result);
#ifdef RESTART_AFTER_ZM_FAILURE
            printf("\r\nRestarting\r\n");
            goto start;
#else        
            printf("stopping\r\n");
            while(1);
#endif
        }
        toggleLed(1);
        delayMs(2000);
    }
}


/* @} */
