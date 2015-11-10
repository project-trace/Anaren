/**
* @ingroup apps
* @{
*
* @file example_simple_application_router_afzdo.c
*
* @brief Resets Module, configures this device to be a Zigbee Router, joins a network, then sends a 
* message to the coordinator periodically with information from the peripherals.
* 
* Demonstrates using a state machine with the Module, and sending sensor values as key-value pairs.
* If you would like to add your own sensor data to the messages, it's very easy:
* - First, determine the name of your object identifier (e.g. OID_HUMIDITY_PERCENT) and also what the value
* represents. Since the value field is just a 16bit integer, sometimes it is best to send the actual
* value / 100 or something similar. For example, when we send temperature we send it as centidegrees
* which allows us 0.01C accuracy. To keep your sanity I recommend including the units in the OID name.
* - Add your new OID to the files Messages/oids.c and Messages/oids.h
* - Write the sensor driver code. I like to actually create a small example that just reads the 
* sensor and displays it to the screen (like the examples included in this code package). This can
* be very helpful when all hell breaks loose; you can go back to your simple example and verify that
* the driver and sensor are all working correctly.
* - Decide whether the sensor will be mains powered or battery powered. If mains powered then make
* it a router so that it can route other messages. If battery powered then it must be an End Device
* - Add your sensor to the appropriate place in getSensorValues() in module_example_utils.c or you 
* can also just add it into this file. 
* - Don't change the coordinator until you get the Router or End Device code working that reads the
* sensor and sends the value. The Simple Application - Coordinator example is useful for debugging 
* in this phase, since it will display any OIDs that are received as well as the name of the OID.
* - Once you get the Router or End Device sending the sensor data properly then work on the other
* end, for example the Coordinator. Often you either want to parse the data and display it, or store
* it in a database table. Both are fairly easy. Most errors are caused by not using consistent units
* in the various places - for example, you might send over the air deciVolts but the database schema
* was created assuming this value is Volts.
* - If you need assistance we're here to help; see below for support options. Tesla has extensive
* experience with sensor-to-server applications and everything inbetween.
*
* @note This matches example_simple_application_coordinator.c
* @note stack size increased to be able to do TMP006 calculations - see project settings
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
#include "Messages/kvp.h"
#include "Messages/oids.h"
#include "module_example_utils.h"
#include <stdint.h>
#include <string.h>  

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

 /* An application-level sequence number to track acknowledgements from server */
uint16_t sequenceNumber = 0;  
 
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

//struct infoMessage im;
struct header hdr;

int main( void )
{
    halInit();
    moduleInit();
    printf("\r\n****************************************************\r\n");    
    printf("Simple Application Example - ROUTER\r\n");
    
    uint16_t vlo = calibrateVlo();
    printf("VLO = %u Hz\r\n", vlo);   
    clearLeds();
    timerIsr = &handleTimer;    
    HAL_ENABLE_INTERRUPTS();
    
    /* Create the infoMessage. Most of these fields are the same, so we can pre-populate most fields.
    These fields are entirely optional, and are just used for our application. This header is based
    on what we use for typical Zigbee deployments. See header.h for description of header fields. */
    hdr.sequence = 0;
    hdr.version = INFO_MESSAGE_VERSION;
    hdr.flags = INFO_MESSAGE_FLAGS_NONE;
    
    initializeSensors();
    delayMs(100);
    stateMachine();    //run the state machine
}

void stateMachine()
{
    while (1)
    {
        if (zigbeeNetworkStatus == NWK_ONLINE)
        {
            if(moduleHasMessageWaiting())      //wait until SRDY goes low indicating a message has been received.   
                displayMessages();
        }
        
        switch (state)
        {
        case STATE_IDLE:
             /* This is the default state, and where we will be spending most time. */
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
             /* Start the module on the network */
            {
#define MODULE_START_DELAY_IF_FAIL_MS 5000
                clearLed(ON_NETWORK_LED);
                moduleResult_t result;
                struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_ROUTER;
                defaultConfiguration.panId = ANY_PAN;
                while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
                {
                    setLed(NETWORK_FAILURE_LED);          // Turn on the LED to show failure
                    printf("FAILED. Error Code 0x%02X. Retrying...\r\n", result);
                    delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);                    
                    clearLed(NETWORK_FAILURE_LED);
                    delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);
                }

                setLed(ON_NETWORK_LED);  // Indicate we got on the network
                printf("Success\r\n"); 
                zigbeeNetworkStatus = NWK_ONLINE;
                /* Module Initialized so we can now store the module's MAC Address in the header */
                zbGetDeviceInfo(DIP_MAC_ADDRESS);
                memcpy(hdr.mac, zmBuf+SRSP_DIP_VALUE_FIELD, 8);
                
                /* Now that we're on the network it's safe to enable message timer: */
                int16_t timerResult = initTimer(2);
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
                if ((sysGpio(GPIO_SET_DIRECTION, ALL_GPIO_PINS) != MODULE_SUCCESS) || (sysGpio(GPIO_CLEAR, ALL_GPIO_PINS) != MODULE_SUCCESS))
                {
                    printf("ERROR\r\n");
                }
                state = STATE_SEND_INFO_MESSAGE;
                break;   
            }
        case STATE_SEND_INFO_MESSAGE:
            {
                printf("~im~");
                struct infoMessage im;
                /* See infoMessage.h for description of these info message fields.*/
                im.header = hdr;    // deep copy
                im.deviceType = DEVICETYPE_TESLA_CONTROLS_ROUTER_DEMO;
                hdr.sequence = sequenceNumber++;
                clearLed(ON_NETWORK_LED);                    // Note: to prevent polluting the color sensor with our colored LEDs, we turn off the LEDs in getSensorValues()
                im.numParameters = getSensorValues(im.kvps);    // Does two things: Loads infoMessage with sensor value KVPs and gets the number of them
                setLed(ON_NETWORK_LED);     // Restore nwk LED setting
                setLed(SEND_MESSAGE_LED); //indicate that we are sending a message
                printInfoMessage(&im);
#define RESTART_DELAY_IF_MESSAGE_FAIL_MS 5000
                uint8_t* messageBuffer = (zmBuf + 100); //to conserve RAM we use the last bytes of zmBuf.
                serializeInfoMessage(&im, messageBuffer);                           //Convert our message struct to an array of bytes
                moduleResult_t result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, 0, INFO_MESSAGE_CLUSTER, messageBuffer, getSizeOfInfoMessage(&im)); // and send it
                clearLed(SEND_MESSAGE_LED);
                if (result != MODULE_SUCCESS)
                {
                    zigbeeNetworkStatus = NWK_OFFLINE;
                    printf("afSendData error 0x%02X; restarting...\r\n", result);
                    delayMs(RESTART_DELAY_IF_MESSAGE_FAIL_MS);  //allow enough time for coordinator to fully restart, if that caused our problem
                    state = STATE_MODULE_STARTUP;
                } else {   
                    printf("Success\r\n");
                    state = STATE_IDLE;
                }
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
    printf("$");   
    stateFlags |= STATE_FLAG_SEND_INFO_MESSAGE;
}


/* @} */
