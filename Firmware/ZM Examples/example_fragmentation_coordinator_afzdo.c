/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_fragmentation_coordinator_afzdo.c
 *
 * @brief Resets Module, configures this device to be a Zigbee Coordinator, and displays any messages
 * that are received. If the message is extended then gets all the parts and displays it.
 * Uses the AF/ZDO interface.
 *
 * This matches example_fragmentation_ROUTER.c
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
#include "../ZM/zm_phy.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../ZM/zm_phy_spi.h"
#include "../Common/utilities.h"
#include <string.h>
#include "module_example_utils.h"

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

/* Use a smaller message size if we're compiling for Launchpad. Launchpad cannot handle a full 600B 
 message because the MSP430F2553 only has 512B of RAM. The other platforms (Stellaris, MDB1, etc.)
 use processors with more RAM and can support a longer message. */
#ifdef LAUNCHPAD
#define MESSAGE_LENGTH     130
#else
#define MESSAGE_LENGTH     AF_DATA_REQUEST_EXT_MAX_TOTAL_PAYLOAD_LENGTH
#endif

uint8_t totalMessage[MESSAGE_LENGTH];  //for extended messages

moduleResult_t result;

int main( void )
{
    halInit();
    moduleInit();    
    printf("\r\n****************************************************\r\n");
    printf("Fragmentation Example - COORDINATOR - using AFZDO\r\n");
    HAL_ENABLE_INTERRUPTS();
    setLed(0);

#define MODULE_START_DELAY_IF_FAIL_MS 5000
    /* See basic communications examples for more information about module startup. */
    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_COORDINATOR;
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS);
    }
    printf("On Network!\r\n");    
    setLed(1);

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#endif  

    /* Now the network is running - continually poll for any received messages from the Module */
    while (1)
    {
        /** First, if there is a message waiting, then retrieve it. */
        if (MODULE_HAS_MESSAGE_WAITING())        
            getMessage();
        if (zmBuf[SRSP_LENGTH_FIELD] > 0)
        {
            /* Display the raw contents of the message buffer */
            printf("Rx: ");
            printHexBytes(zmBuf, (zmBuf[SRSP_LENGTH_FIELD] + SRSP_HEADER_SIZE));
            /* If it is an extended message */
            if (IS_AF_INCOMING_MESSAGE_EXT()) 
            {
                uint16_t len = AF_INCOMING_MESSAGE_EXT_LENGTH();
                printf("Extended Message Received, L%u ", len);
                if (len > MESSAGE_LENGTH)
                {
                    printf("ERROR: Extended message length is larger than our buffer; ignoring message\r\n");   // If sender is sending more bytes than our buffer can hold
                } else {
                    if (len > AF_DATA_REQUEST_EXT_MAX_PAYLOAD_LENGTH)
                    {
                        /* The message payload is larger than what can fit in one message.
                 Must use helper method to get the full payload. */
                        printf(" Retrieving extended message contents\r\n");
                        /* Use our very cool utility method to fetch the entire payload from the module
                     and copy it into the totalMessage buffer. This makes multiple calls to the module */
                        moduleResult_t result = retrieveExtendedMessage(zmBuf + AF_INCOMING_MESSAGE_EXT_TIMESTAMP_START_FIELD, len, totalMessage);
                        if (result == MODULE_SUCCESS)
                            printHexBytes(totalMessage, len);
                        else
                            printf("retrieveExtendedMessage error %02X\r\n");

                    } else {
                        /* One message holds the entire message payload so we can get contents directly */
                        printf(" All in one message\r\n");
                        memcpy(totalMessage, zmBuf+AF_INCOMING_MESSAGE_EXT_PAYLOAD_START_FIELD, len);
                        /* totalMessage holds entire message now */
                        printHexBytes(totalMessage, len);
                    }
                }
            }
            zmBuf[SRSP_LENGTH_FIELD] = 0;
        }
    }
}


/* @} */
