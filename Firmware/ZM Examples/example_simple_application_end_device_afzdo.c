/**
* @ingroup apps
* @{
*
* @file example_simple_application_end_device_afzdo.c
*
* @brief Resets Module, configures this device to be a Zigbee End Device, joins a network, then sends a 
* message to the coordinator periodically with information from the peripherals.
* 
* Demonstrates using a state machine with the Module. 
* @see  example_simple_application_router.c for instructions on how to add additional sensor data
*
* @note This matches example_simple_application_coordinator.c
* @note This example will not compile with the IAR Kickstart edition because CODE + CONST exceeds 4kB
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
#include "../ZM/zm_phy.h"
#include "../Common/utilities.h"
#include "Messages/infoMessage.h"
#include "module_example_utils.h"
#include <stdint.h>
#include <string.h>  

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

uint16_t sequenceNumber = 0;  //an application-level sequence number to track acknowledgements from server

/** function pointer (in hal file) for the function that gets called when the timer generates an int*/
extern void (*timerIsr)(void);

/** STATES for state machine */
enum STATE
{
    STATE_IDLE,
    STATE_MODULE_STARTUP,
    STATE_SEND_INFO_MESSAGE,
    STATE_DISPLAY_NETWORK_INFORMATION,
};

/** This is the current state of the application. 
Gets changed by other states, or based on messages that arrive. */
enum STATE state = STATE_MODULE_STARTUP;

/** The main application state machine */
void stateMachine();

/** Various flags between states */
uint16_t stateFlags = 0;
#define STATE_FLAG_SEND_INFO_MESSAGE 0x01

/** Handles timer interrupt */
void handleTimer();

#define NWK_OFFLINE                     0
#define NWK_ONLINE                      1
/** Whether the zigbee network has been started, etc.*/
uint8_t zigbeeNetworkStatus = NWK_OFFLINE;

struct header hdr;

int main( void )
{
    halInit();
    moduleInit();    
    HAL_DISABLE_INTERRUPTS();
    printf("\r\n****************************************************\r\n");    
    printf("Simple Application Example - END DEVICE\r\n");  
    uint16_t vlo = calibrateVlo();
    printf("VLO = %u Hz\r\n", vlo);   
    timerIsr = &handleTimer;  
    clearLeds();
    HAL_ENABLE_INTERRUPTS();
    
    /* Most of the of infoMessage are the same, so we can create most of the message ahead of time. */
    hdr.sequence = 0;  //this will be incremented each message
    hdr.version = INFO_MESSAGE_VERSION;
    hdr.flags = INFO_MESSAGE_FLAGS_NONE;

    initializeSensors();
    delayMs(100);
    stateMachine();
}

void stateMachine()
{
    while (1)
    {
        if (zigbeeNetworkStatus == NWK_ONLINE)
        {
            if(MODULE_HAS_MESSAGE_WAITING())      //wait until SRDY goes low indicating a message has been received.   
                displayMessages();
        }
        
        switch (state)
        {
        case STATE_IDLE:
            {
                if (stateFlags & STATE_FLAG_SEND_INFO_MESSAGE)  //if there is a pending info message to be sent
                {
                    state = STATE_SEND_INFO_MESSAGE;            //then send the message and clear the flag
                    stateFlags &= ~STATE_FLAG_SEND_INFO_MESSAGE;
                }
                /* Other flags (for different messages or events) can be added here */
                break;
            }
            
        case STATE_MODULE_STARTUP:
            {
#define MODULE_START_DELAY_IF_FAIL_MS   5000    // Must be greater than MODULE_START_FAIL_LED_ONTIME
                clearLed(ON_NETWORK_LED);
                moduleResult_t result;
                struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_END_DEVICE;
                defaultConfiguration.panId = ANY_PAN;                
                while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
                {
                    setLed(NETWORK_FAILURE_LED);          // Turn on the LED to show failure
                    printf("FAILED. Error Code 0x%02X. Retrying...\r\n", result);
                    delayMs(MODULE_START_DELAY_IF_FAIL_MS);
                }
                clearLed(NETWORK_FAILURE_LED);
                setLed(ON_NETWORK_LED);  // Indicate we got on the network
                printf("Success\r\n"); 
                /* Module is now initialized so store MAC Address */
                zbGetDeviceInfo(DIP_MAC_ADDRESS);
                memcpy(hdr.mac, zmBuf+SRSP_DIP_VALUE_FIELD, 8);
                int16_t timerResult = initTimer(4);
                if (timerResult != 0)
                {
                    printf("timerResult Error %d, STOPPING\r\n", timerResult);
                    while (1);   
                }
                
                state = STATE_DISPLAY_NETWORK_INFORMATION;
                break;
            }
        case STATE_DISPLAY_NETWORK_INFORMATION:
            {
                printf("~ni~");
                /* On network, display info about this network */              
                displayNetworkConfigurationParameters();                
                displayDeviceInformation();
                /* Set module GPIOs as output and turn them off */
                if ((sysGpio(GPIO_SET_DIRECTION, ALL_GPIO_PINS) != MODULE_SUCCESS) || (sysGpio(GPIO_CLEAR, ALL_GPIO_PINS) != MODULE_SUCCESS))
                {
                    printf("ERROR\r\n");
                }
                clearLed(ON_NETWORK_LED); //turn off to save power
                state = STATE_SEND_INFO_MESSAGE;
                break;   
            }
        case STATE_SEND_INFO_MESSAGE:
            {
                printf("~im~");

                struct infoMessage im;
                /* See infoMessage.h for description of these info message fields.*/
                im.header = hdr;
                im.deviceType = DEVICETYPE_TESLA_CONTROLS_ROUTER_DEMO;
                im.header.sequence = sequenceNumber++;
                im.numParameters = getSensorValues(im.kvps);    // Does two things: Loads infoMessage with sensor value KVPs and gets the number of them
                /* To prevent polluting the color sensor with our colored LEDs, we don't toggle the LEDs until AFTER getSensorValues() */
                setLed(SEND_MESSAGE_LED); //indicate that we are sending a message
                printInfoMessage(&im);
#define RESTART_DELAY_IF_MESSAGE_FAIL_MS 5000
                uint8_t messageBuffer[MAX_INFO_MESSAGE_SIZE];
                serializeInfoMessage(&im, messageBuffer);
                moduleResult_t result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, 0, INFO_MESSAGE_CLUSTER, messageBuffer, getSizeOfInfoMessage(&im)); // and send it
                if (result != MODULE_SUCCESS)
                {
                    zigbeeNetworkStatus = NWK_OFFLINE;
                    printf("afSendData error %02X; restarting...\r\n", result);
                    clearLed(ON_NETWORK_LED);
                    delayMs(RESTART_DELAY_IF_MESSAGE_FAIL_MS);  //allow enough time for coordinator to fully restart, if that caused our problem
                    state = STATE_MODULE_STARTUP;
                } else {   
                    printf("Success\r\n");
                    state = STATE_IDLE;
                }
                clearLed(SEND_MESSAGE_LED);
                break;   
            }
        default:     //should never happen
            {
                printf("UNKNOWN STATE\r\n");
                state = STATE_MODULE_STARTUP;
            }
            break;
        }
    } 
}

/** Handles timer interrupt */
void handleTimer()
{
    printf(" T");   
    stateFlags |= STATE_FLAG_SEND_INFO_MESSAGE;
}



/* @} */

