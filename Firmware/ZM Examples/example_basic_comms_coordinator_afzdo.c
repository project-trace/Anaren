/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_basic_comms_coordinator_afzdo.c
 *
 * @brief Resets Module, configures this device to be a Zigbee Coordinator, and displays any messages
 * that are received. Uses the AF/ZDO interface.
 *
 * This matches example_basic_communications_ROUTER.c
 * Get this running before the router, or else the router won't have anything to join to.
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
 * YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� 
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
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../ZM/zm_phy.h"
#include "module_example_utils.h"
#include <stdint.h>
#include <stddef.h>

moduleResult_t result;

int main( void )
{
    halInit();
    moduleInit();
    printf("\r\n****************************************************\r\n");
    printf("Basic Communications Example - COORDINATOR - using AFZDO\r\n");
    HAL_ENABLE_INTERRUPTS();
    setLed(0);

#define MODULE_START_DELAY_IF_FAIL_MS 5000
    /* Use the default module configuration */

    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_COORDINATOR;
    /* Change this if you are using a custom PAN */
    defaultConfiguration.panId = ANY_PAN;
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        /* Module startup failed; display error */
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS);
    }
    printf("Success\r\n"); 

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif  
    printf("Displaying Received Messages...\r\n");

    while (1)
    {
        /* Continually loop, displaying any messages that are received */
        if (MODULE_HAS_MESSAGE_WAITING())        
            pollAndDisplay();
    }
}

/* @} */
