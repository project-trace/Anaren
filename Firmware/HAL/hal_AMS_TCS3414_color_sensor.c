/**
* @ingroup hal
* @{ 
* @file hal_AMS_TCS3414_color_sensor.c
*
* @brief Interface driver to AMS/TAOS TCS3414 color sensor.
*
* @section Usage
* To use this driver in an application, include the following code:
<pre>
    halInit();
    colorSensorInit();
    struct ColorReading color;
    color = getColor();
    printf("\r\nRED: %05u - BLUE: %05u - GREEN: %05u - CLEAR: %05u\r\n", color.red, color.blue, color.green, color.clear);   
</pre>
*
* @see http://www.ams.com/eng/Products/Light-Sensors/Color-Sensor/TCS3414
*
* $Rev: 1528 $
* $Author: dsmith $
* $Date: 2012-08-30 15:58:08 -0700 (Thu, 30 Aug 2012) $
*
* @section support Support
* Please refer to the wiki at www.anaren.com/air-wiki-zigbee for more information. Additional support
* is available via email at the following addresses:
* - Questions on how to use the product: AIR@anaren.com
* - Feature requests, comments, and improvements:  featurerequests@teslacontrols.com
* - Consulting engagements: sales@teslacontrols.com
*
* @section license License
* Copyright (c) 2012 Anaren Microwave. All rights reserved. This Software may only be used with an 
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

#include "hal.h"
#include "hal_AMS_TCS3414_color_sensor.h"
#include "../Common/utilities.h"
#include <stdint.h>
#include <math.h>

//
//  Private Methods
//

/** Reads the 8-bit value of a single 8 bit register from the color sensor
@pre I2C bus was initialized with i2cInit(COLOR_SENSOR_I2C_ADDRESS)
@param commandRegister which register to write to
@return the value of the register
@note could be implemented using i2cBlockRead too
*/
uint8_t readByte(uint8_t commandRegister)
{
    unsigned char I2CBuffer[2];
    I2CBuffer[0] = commandRegister;  
    i2cWrite(I2CBuffer, 1);
    uint8_t result = i2cReadOneByte(I2CBuffer);
    if (result != 0)
    {
        printf("TCS3414 readByte ");
        displayI2cError(result);
    }
#ifdef TCS3414_VERBOSE
    printf("Read %02X from %02X\r\n", I2CBuffer[0], commandRegister);
#endif        
    return (I2CBuffer[0]);
}

/** Writes the value of a single 8 bit register in the color sensor
@pre I2C bus was initialized with i2cInit(COLOR_SENSOR_I2C_ADDRESS)
@param commandRegister which register to write to
@param value the value to be written to the register
*/
void writeByte(uint8_t commandRegister, uint8_t value)
{
#ifdef TCS3414_VERBOSE
    printf("Write %02X to %02X\r\n", value, commandRegister);
#endif
    unsigned char I2CBuffer[2];  //First byte is cmd reg, next two bytes are for address high/low
    I2CBuffer[0] = commandRegister;           // store single bytes that have to be sent
    I2CBuffer[1] = value;
    uint8_t result = i2cWrite(I2CBuffer, 2);
    if (result != 0)
    {
        printf("TCS3414 writeByte ");
        displayI2cError(result);
    }
}

/** Reads the 16-bit value of a single 8 bit register from the color sensor
@pre I2C bus was initialized with i2cInit(COLOR_SENSOR_I2C_ADDRESS)
@param commandRegister which register to write to
@return the value of the register
*/
uint16_t readWord(uint8_t commandRegister, uint8_t readAddress)
{
    unsigned char I2CBuffer[2];  // will be used for reading and writing
    uint8_t result = 0;

    I2CBuffer[0] = commandRegister;  
    I2CBuffer[1] = readAddress;      
    result = i2cWrite(I2CBuffer, 2);
    if (result != 0)
    {
        printf("TCS3414 readWordw ");
        displayI2cError(result);
        return 0;
    }

    result = i2cRead(I2CBuffer, 2);
    if (result != 0)
    {
        printf("TCS3414 readWordr ");
        displayI2cError(result);
        return 0;
    }

    uint8_t lsb = I2CBuffer[0];      
    uint8_t msb = I2CBuffer[1];
#ifdef TCS3414_VERBOSE
    printf("Read %04X from register %02X\r\n", CONVERT_TO_INT(lsb, msb), commandRegister);
#endif
    return (CONVERT_TO_INT(lsb, msb));
}

//
//  Public Methods
//

/** Configures the color sensor. Turns it on and configures the integration time. Also reads back the ID register.
@pre I2C interface on microcontroller was configured correctly
@note Part maximum baud rate 400kHz for I2C.
@note On powerup, the sensor is powered down and needs to be turned on before use.
*/
void colorSensorInit()   
{
    i2cInit(COLOR_SENSOR_I2C_ADDRESS);
    printf("Color Sensor Initialize\r\n");
    delayMs(1);

    // Turn off TCS3414. Not technically needed but good to reset everything to defaults. On I2C this is W 0x72, 0x80, 0x00
    writeByte(TCS3414_REGISTER_CONTROL | TCS3414_COMMAND_BIT, 0);
    delayMs(100);	// wait a little bit

    // Turn on the part: CMD bit is 0x80, Control is 0x00, poweron is 0x03. On I2C this is W 0x72, 0x80, 0x01
    writeByte(TCS3414_REGISTER_CONTROL | TCS3414_COMMAND_BIT, TCS3414_CONTROL_POWERON);

    // Set integration time, timing is 0x01, 100mSec is 0x01. On I2C this is W 0x72, 0x81, 0x01
    writeByte(TCS3414_REGISTER_TIMING | TCS3414_COMMAND_BIT, TCS3414_INTEGRATION_TIME_100MS);

    // Set ADC gain to 4X
    writeByte(TCS3414_REGISTER_GAIN | TCS3414_COMMAND_BIT, TCS3414_GAIN_4X);

    // CMD bit is 0x80, RevId is 0x04. On I2C this is W 0x72, 0x84; R 0x73, 0x11 + NAK.
    printf("    ID Register = 0x%02X\r\n", readByte(TCS3414_REGISTER_PARTNO_REVID | TCS3414_COMMAND_BIT));

    // Turn on the ADC. Must be done AFTER the integration time is set. On I2C this is W 0x72, 0x80, 0x03
    writeByte(TCS3414_REGISTER_CONTROL | TCS3414_COMMAND_BIT, (TCS3414_CONTROL_POWERON | TCS3414_CONTROL_ADC_EN));
}

/**
Converts RGB values to Correlated Color Temperature (CCT). CCT is measured in Kelvin (K) and
describes whether the ambient light is more warm (low CCT) or cool (high CCT).
Typical CCT values are:
 - Incandescent lamp: 3200K
 - Cool White fluorescent lamp: 5000
 - Overcast sky: 6500
@see http://en.wikipedia.org/wiki/Color_Temperature
@see "Calculating Color Temperature and Illuminance using the TAOS TCS3414 Digital Color Sensor"
@return color temperature in K
*/
uint16_t convertRGBToCCT(uint16_t red, uint16_t green, uint16_t blue)
{
    // First, convert each color to a percentage:
    float R = (((float) red / UINT16_MAX) / 100.0);
    float G = (((float) green / UINT16_MAX) / 100.0);
    float B = (((float) blue / UINT16_MAX) / 100.0);

    // Now, convert the percentages to the CIE trisimulus values X, Y, Z
    float X = (-0.14282) * (R) + (1.54924) * (G) + (-0.95641) * (B);
    float Y = (-0.32466) * (R) + (1.57837) * (G) + (-0.73191) * (B);  // Note: Y is illuminance
    float Z1 = (-0.68202) * (R) + (0.77073) * (G) + (0.56332) * (B);

    // Calculate chromaticity coordinates x,y
    float xx = (X)/(X+Y+Z1);
    float yy = (Y)/(X+Y+Z1);

    // Use McCamy's formula to compute the CCT from the chromaticity coordinates
    float n = (xx - 0.3320) / (0.1858 - yy);
    uint16_t cct = (uint16_t) ((449.0 * pow(n , 3.0)) + (3525.0 * pow(n, 2.0)) + 5520.33);

    return cct;
}

/** Read color values from the tcs3414 color sensor.
@return ColorReading structure containing values for Red, Green, Blue, and Clear sensor readings.
@pre I2C bus was configured properly and color sensor was initialized with colorSensorInit().
*/
struct ColorReading getColor(void)
{
    struct ColorReading color;

    /* Now, initialize the sensor. Required if we're sharing the bus with other peripherals */
    i2cInit(COLOR_SENSOR_I2C_ADDRESS); 

    /* Read each color value */
    color.red = readWord(TCS3414_REGISTER_REDLOW | TCS3414_COMMAND_BIT | TCS3414_WORD_BIT, TCS3414_READ_ADDRESS);
    color.green = readWord(TCS3414_REGISTER_GREENLOW | TCS3414_COMMAND_BIT | TCS3414_WORD_BIT, TCS3414_READ_ADDRESS);
    color.blue = readWord(TCS3414_REGISTER_BLUELOW | TCS3414_COMMAND_BIT | TCS3414_WORD_BIT, TCS3414_READ_ADDRESS);
    color.clear = readWord(TCS3414_REGISTER_CLEARLOW | TCS3414_COMMAND_BIT | TCS3414_WORD_BIT, TCS3414_READ_ADDRESS);

    return color;
}

/* @} */
