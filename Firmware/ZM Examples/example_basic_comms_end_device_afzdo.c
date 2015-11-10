/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_basic_comms_end_device_afzdo.c
 *
 * @brief Resets Radio, configures this device to be a Zigbee End Device, joins a network,
 * then sends a message to the coordinator every few seconds. Sleeps inbetween transmissions.
 * Will also transmit a message when the button is pressed.
 *
 * Calibrates the Very-Low-Power Oscillator (VLO) in the MSP430. Uses VLO to wakup the processor.
 * Note: you could also use an external 32kHz xtal for more accuracy/stability
 *
 * Uses the AF/ZDO interface.
 *
 * This matches example_basic_communications_coordinator.c
 * Get the coordinator running first, or else the end device won't have anything to join to.
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
#include "module_example_utils.h"

/** function pointer (in hal file) for the function that gets called when the timer generates an int*/
extern void (*timerIsr)(void);

/** function pointer (in hal file) for the function that gets called when a button is pressed*/
extern void (*buttonIsr)(int8_t);

/** Used to store return value from module operations */
moduleResult_t result;

/** Handles timer interrupt */
void handleTimer();

/** Handles button interrupt */
void handleButtonPress(int8_t whichButton);

#define RESTART_AFTER_ZM_FAILURE

uint8_t counter = 0;
uint8_t testMessage[] = {0xA0,0xA1,0xA2,0xA3,0xA4};

/* This example may be used to measure the module's sleep current. To do this, first define the 
compile option DISABLE_END_DEVICE_POLLING. In IAR, this can be done in Project Options : C/C++ compiler : Preprocessor
In the defined symbols box, add:
DISABLE_END_DEVICE_POLLING
This will prevent the end device from waking up every second or so and checking for any new messages.
This must be defined project-wide because it affects other files. It will not allow the end device 
to receive messages though. Second, undefine SEND_MESSAGE_ON_TIMER in this file to not use the timer. 
This will stop the module from communicating and will allow you to measure sleep current or when you 
want to manually wake up (e.g. on a button). */

#define SEND_MESSAGE_ON_TIMER

int main( void )
{
    halInit();
    moduleInit();     
    printf("\r\n****************************************************\r\n");    
    printf("Basic Communications Example - END DEVICE - using AFZDO\r\n");
    /* Configure the buttonIsr (called from the hal file) to point to our handleButtonPress() */
    buttonIsr = &handleButtonPress;   
    halSetWakeupFlags(WAKEUP_AFTER_BUTTON);

    /** Calibrate the Very Low Power Oscillator (VLO) to use for sleep timing */
    printf("Calibrating VLO...\r\n");

    int16_t vlo = -1;
    while (vlo < 0)
    {
        vlo = calibrateVlo();
        printf("VLO = %u Hz; ", vlo);
    }
    printf(" Done.\r\n");
    timerIsr = &handleTimer; 
    HAL_ENABLE_INTERRUPTS();   

#define MODULE_START_DELAY_IF_FAIL_MS 5000
    /* Use the default module configuration */

    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_END_DEVICE;
    /* Change this if you are using a custom PAN */
    defaultConfiguration.panId = ANY_PAN;
    start:
    /* Turn Off nwk status LED if on */
    clearLed(ON_NETWORK_LED);
    /* Loop until module starts */      
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        /* Module startup failed; display error and blink LED */
        setLed(NETWORK_FAILURE_LED);                    
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);

        delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);                    
        clearLed(NETWORK_FAILURE_LED);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);
    }
    printf("On Network!\r\n");
    setLed(ON_NETWORK_LED);  // Indicate we got on the network

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif  

    /* Now the network is running - send a message to the coordinator every few seconds.*/

#define TEST_CLUSTER 0x77

#ifdef SEND_MESSAGE_ON_TIMER
    int16_t timerResult = initTimer(4);
    if (timerResult != 0)
    {
        printf("timerResult Error %i, STOPPING\r\n", timerResult);
        while (1);   
    }
#endif

    while (1)
    {
        /* Indicate that we are sending a message */
        setLed(SEND_MESSAGE_LED);
        printf("Sending Message %u  ", counter++);
        /* Attempt to send the message */
        result = afSendData(DEFAULT_ENDPOINT,DEFAULT_ENDPOINT,0, TEST_CLUSTER, testMessage, 5);
        clearLed(SEND_MESSAGE_LED);
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
        HAL_SLEEP();
    }
}

void handleTimer()  //Will automatically wakeup timer
{
    printf("#");
}

void handleButtonPress(int8_t whichButton)
{
    toggleLed(0);
    printf("Button Pressed\r\n");
}

/* @} */
