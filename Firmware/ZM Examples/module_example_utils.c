/**
 * @file module_example_utils.c
 *
 * @brief Simple utilities for Module examples.
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
#include "../ZM/af.h"   
#include "../ZM/zdo.h"    
#include "../ZM/zm_phy.h"
#include "../Common/utilities.h"
#include "Messages/oids.h"
#include "Messages/kvp.h"
#include "module_example_utils.h"
#include "../HAL/hal_AMS_TCS3414_color_sensor.h"
#include "../HAL/hal_TI_TMP006_IR_temperature_sensor.h"
#include <math.h>

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

/** Poll the Module for any messages and display them to the console.
Used in Simple API coordinator, AFZDO coordinator, secure comms examples, etc.
@pre moduleHasMessageWaiting() returned true.
 */
void pollAndDisplay()
{
    getMessage();
    if (zmBuf[SRSP_LENGTH_FIELD] > 0)
    {
        setLed(1);
        printf("Rx: ");
        printHexBytes(zmBuf, (zmBuf[SRSP_LENGTH_FIELD] + SRSP_HEADER_SIZE));
        zmBuf[SRSP_LENGTH_FIELD] = 0;
        clearLeds();
    } 
}

/** Initializes any sensors used in the application. Device dependent, since different platforms
     have different sensors. */
void initializeSensors()
{
#ifdef INCLUDE_IR_TEMPERATURE_SENSOR
    tmp006Init();   
#endif
#ifdef INCLUDE_COLOR_SENSOR
    colorSensorInit();
#endif
}

/** Retrieves all the sensor values for this device. This is device dependent.
@param kvps an array of kvps. This will be filled in by this method, starting at index 0.
@pre kvps is large enough to hold all the sensors on this device.
@return the number of sensors
 */
int8_t getSensorValues(struct kvp *kvps)
{
    printf("\r\n");
    int8_t kvpsIndex = 0;
#if defined LAUNCHPAD
    kvps[kvpsIndex].oid = OID_SUPPLY_VOLTAGE_MV;
    int16_t value = getVcc3();
    kvps[kvpsIndex++].value = value;
    printf("Supply Voltage = %d.%03dV\r\n", value/1000, (value%1000));

#ifdef INCLUDE_IR_TEMPERATURE_SENSOR
    kvps[kvpsIndex].oid = OID_TEMPERATURE_IR;
    struct TempReading currTemp;
    tmp006GetTemperature(&currTemp);
    delayMs(1);
    printf("IR Temperature = %d.%02dC\r\n", currTemp.tempInt/100, (currTemp.tempInt%100));
    kvps[kvpsIndex++].value = currTemp.tempInt;  //NOTE: This is temp * 100
#endif
#ifdef INCLUDE_COLOR_SENSOR
    struct ColorReading color = getColor();
    printf("RED: %u - GREEN: %u - BLUE: %u - CLEAR: %u\r\n", color.red, color.green, color.blue, color.clear);

    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_RED;
    kvps[kvpsIndex++].value = color.red;
    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_GREEN;
    kvps[kvpsIndex++].value = color.green;
    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_BLUE;
    kvps[kvpsIndex++].value = color.blue;
    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_CLEAR;
    kvps[kvpsIndex++].value = color.clear;
#endif
#elif defined MDB1
    kvps[kvpsIndex].oid = OID_SUPPLY_VOLTAGE_MV;
    int16_t value = getVcc3();
    kvps[kvpsIndex++].value = value;
    printf("Supply Voltage = %d.%03dC\r\n", value/1000, (value%1000));

#elif defined EK_LM4F120XL
    /** Note: this Stellaris processor cannot measure its own supply voltage so we don't include OID_SUPPLY_VOLTAGE_MV */
#ifdef INCLUDE_COLOR_SENSOR
    struct ColorReading color = getColor();
    printf("RED: %u - GREEN: %u - BLUE: %u - CLEAR: %u\r\n", color.red, color.green, color.blue, color.clear);

    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_RED;
    kvps[kvpsIndex++].value = color.red;
    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_GREEN;
    kvps[kvpsIndex++].value = color.green;
    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_BLUE;
    kvps[kvpsIndex++].value = color.blue;
    kvps[kvpsIndex].oid = OID_COLOR_SENSOR_CLEAR;
    kvps[kvpsIndex++].value = color.clear;
#endif
#ifdef INCLUDE_IR_TEMPERATURE_SENSOR
    kvps[kvpsIndex].oid = OID_TEMPERATURE_IR;
    struct TempReading currTemp;
    tmp006GetTemperature(&currTemp);
    delayMs(1);
    printf("IR Temperature = %d.%02dC\r\n", currTemp.tempInt/100, (currTemp.tempInt%100));
    kvps[kvpsIndex++].value = currTemp.tempInt;  //NOTE: This is temp * 100
#endif
#else
#error "getSensorValues in module_example_utils.c requires a board defined."
#endif
    return kvpsIndex;
}

/** Reads all sensors and displays them */
void testSensors()
{
    struct kvp kvps[MAX_SENSORS];  //will hold sensor values
    int8_t numSensors = getSensorValues(kvps);  //populate info message with
    int i = 0;
    printf("%d KVPs:\r\n", numSensors);
    for (i = 0; i< numSensors; i++)
    {
        printf("    #%u: Key=0x%02X, Value=%d\r\n", i, kvps[i].oid, kvps[i].value);
    }
}

#ifdef INCLUDE_RGB_LED // Don't include these variables if not used.

#define RGB_LED_MIDPOINT    0x7F

#define NOMINAL_COLOR_NOT_SET     0.0
#define RED 1
#define GREEN 0
#define BLUE 2

/** Simple utility method that computes the percentages of each color */
static void displayColorPercentages(uint16_t red, uint16_t blue, uint16_t green)
{
    float colorPercent[3] = {0.0};
    float totalColor = red + blue + green;
    colorPercent[RED] = ((float) red) / totalColor;
    colorPercent[GREEN] = ((float) green) / totalColor;
    colorPercent[BLUE] = ((float) blue) / totalColor;

    printf("Red=%02u%%, Green=%02u%%, Blue=%02u%%\r\n", 
            (uint8_t) (colorPercent[RED] * 100),
            (uint8_t) (colorPercent[GREEN] * 100),
            (uint8_t) (colorPercent[BLUE] * 100));
}

/**
Displays a color as measured as percentages on the RGB LED. 
First, if the value is above 1.0 (100%) then the value is limited to 1.0 (100%).
Then, we find which value is max (red, green, or blue). The values that come in may not be scaled;
e.g. they may be 20%, 30%, 40%. But to get maximum luminance we scale these values, setting the
highest value to the maximum PWM output (defined as RGB_LED_PWM_PERIOD). 
@param r red percentage
@param b blue percentage
@param g green percentage
 */
static void displayColorPercentagesOnRgbLed(float r, float b, float g)
{
    //printf("R=%u%%, B=%u%%, G=%u%%\r\n", ((uint8_t) (r * 100)), ((uint8_t) (b * 100)), ((uint8_t) (g * 100)));
    uint8_t outputRed = RGB_LED_PWM_PERIOD;
    uint8_t outputBlue = RGB_LED_PWM_PERIOD;
    uint8_t outputGreen = RGB_LED_PWM_PERIOD;

    // If the value is too high then limit it to 1.0 (100%) maximum
    r = (r > 1.0) ? 1.0 : r;
    b = (b > 1.0) ? 1.0 : b;
    g = (g > 1.0) ? 1.0 : g;

    // Now, find which value is maximum and scale accordingly
    if ((r > b) && (r > g)) {           // Red is max
        outputRed = RGB_LED_PWM_PERIOD;
        outputBlue = (uint8_t) ((b/r) * RGB_LED_PWM_PERIOD);
        outputGreen = (uint8_t) ((g/r) * RGB_LED_PWM_PERIOD);        
    } else if ((b > r) && (b > g)) {    // Blue is max
        outputRed = (uint8_t) ((r/b) * RGB_LED_PWM_PERIOD);
        outputBlue = RGB_LED_PWM_PERIOD;
        outputGreen = (uint8_t) ((g/b) * RGB_LED_PWM_PERIOD);    
    } else if ((g > r) && (g > b)) {    // Green is max
        outputRed = (uint8_t) ((r/g) * RGB_LED_PWM_PERIOD);
        outputBlue = (uint8_t) ((b/g) * RGB_LED_PWM_PERIOD);       
        outputGreen = RGB_LED_PWM_PERIOD;       
    } else {
        // Values already set to RGB_LED_PWM_PERIOD
    }
    printf("Output to RGB LED: Red=0x%02X, Green=0x%02X, Blue=0x%02X", outputRed, outputGreen, outputBlue);
    //displayColorPercentages(outputRed, outputBlue, outputGreen);
    halRgbSetLeds(outputRed, outputBlue, outputGreen); 
}

/* These values have been determined experimentally and represent the offset of the Color Sensor */
#define OFFSET_RED          500
#define OFFSET_GREEN        490
#define OFFSET_BLUE         320

/** 
Displays the received color sensor values on the RGB LED. Corrects for color senosr bias by 
subtracting an emperically derived offset for each color.
@param red the value of the red sensor
@param blue the value of the blue sensor
@param green the value of the green sensor
@note offset values computed with color sensor gain = 4x
 */
void displayColorOnRgbLed(uint16_t red, uint16_t blue, uint16_t green)
{
    printf("Red=%u, Green=%u, Blue=%u (Total %u); ", red, green, blue, (red+green+blue));
    displayColorPercentages(red, blue, green);
    float r = (red > OFFSET_RED) ? (red - OFFSET_RED) : 0;
    float b = (blue > OFFSET_BLUE) ? (blue - OFFSET_BLUE) : 0;
    float g = (green > OFFSET_GREEN) ? (green - OFFSET_GREEN) : 0;
    float tot = r + b + g;

    displayColorPercentagesOnRgbLed(r/tot, b/tot, g/tot);
    return;
}

#define NOMINAL_TEMPERATURE_NOT_SET     0xFF

/** Used for the display of received temperature on the RGB LED, if present. This defaults to 0xFF
until the first packet with temperature data is received, when it is set.*/
int16_t temperatureNominal = NOMINAL_TEMPERATURE_NOT_SET;


/** 
Displays the temperature difference on the RGB LED. Temperature difference is limited to +/- 7
degrees maximum. If nominal temperature has not been set yet then it uses the first value as the
nominal temperature. The Color displayed according to the following rules:
- Within 2 degrees of nominal: Green
- Greater than 2 degrees, display red.
- Less than -2 degrees, display blue.
@param value the temperature in centi-degrees celsius. Divide by 100 to get degrees.
 */
void displayTemperatureOnRgbLed(int16_t value)
{
    if (temperatureNominal == NOMINAL_TEMPERATURE_NOT_SET)
    {
        temperatureNominal = value;
        printf("Setting Nominal Temperature = %d.%02dC\r\n", (value/100), (value%100));
        halRgbSetLeds(RGB_LED_MIDPOINT, RGB_LED_MIDPOINT, RGB_LED_PWM_OFF);
        return;
    }
    //process the temperature difference

    //update the temperature blue/red mix:
    int16_t temperatureDifference = ((value - temperatureNominal) / 100);  //positive means it got warmer
    uint8_t redOnTime = RGB_LED_PWM_OFF;
    uint8_t greenOnTime = RGB_LED_PWM_OFF;     
    uint8_t blueOnTime = RGB_LED_PWM_OFF;   
    if ((temperatureDifference <= 2) && (temperatureDifference >= -2))
    {
        greenOnTime = RGB_LED_PWM_PERIOD;
    } else {

        // Now, value is either 3..7 or -3..-7:

        // limit temperature difference (displayed) to 7 degrees up or down:
        temperatureDifference = (temperatureDifference > 7) ? 7 : temperatureDifference;
        temperatureDifference = (temperatureDifference < -7) ? -7 : temperatureDifference;

        printf("Adjusted temperatureDifference = %d\r\n", temperatureDifference);

        // each degree of Temp Difference (hotter) means that blue onTime decreases by 0x10, red onTime increases by 0x10;
        blueOnTime = RGB_LED_MIDPOINT - (temperatureDifference * 0x10);
        redOnTime = RGB_LED_MIDPOINT + (temperatureDifference * 0x10);

    }

    halRgbSetLeds(redOnTime, blueOnTime, greenOnTime);    
    printf("Temperature = %u.%02uC; %d from nominal. Red=0x%02X, Green=0x%02X, Blue=0x%02X\r\n", 
            (value/100), (value%100), temperatureDifference, redOnTime, greenOnTime, blueOnTime);
}

/** Resets the nominal temperature so that it will be set by the next incoming packet */
void resetNominalTemperature()
{
    printf("Resetting Nominal Temperature\r\n");
    temperatureNominal = NOMINAL_TEMPERATURE_NOT_SET;
}

/** Resets the nominal color so that it will be set by the next incoming packet */
void resetNominalColor()
{  
    // Not used, placeholder in case you want to use a default color.
}

#endif
