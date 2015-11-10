/**
* @ingroup hal
* @{ 
* @file hal_TI_TMP006_IR_temperature_sensor.c
*
* @brief Hardware Abstraction Layer (HAL) file for TI TMP006 Infrared temperature sensor.
*
* This sensor measures the radiated temperature of an object in "view" of the sensor.
* Sensor communicates via I2C.
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
#include "hal_TI_TMP006_IR_temperature_sensor.h"
#include <math.h>
#include "../Common/utilities.h"
#include <stdint.h>

/** Holds tDie of 4 readings for transient correction, with tDie[0] = oldest, tdie[3] = newest. */
static float tDie[4] = {0};

//
//  Private methods
//

/** 
Writes the 16-bit value of a single 8 bit register in the color sensor
@pre I2C bus was initialized with i2cInit(TMP006_I2C_ADDRESS)
@param commandRegister which register to write
@param value the value that will be written to the register
*/
static void i2cWrite16(uint8_t commandRegister, uint16_t value)
{
    unsigned char I2CBuffer[3];  			//First byte is cmd reg, next two bytes are for address high/low
    I2CBuffer[0] = commandRegister;         // store single bytes that have to be sent
    I2CBuffer[1] = (value>>8);
    I2CBuffer[2] = (value&0x0F);
    uint8_t result = i2cWrite(I2CBuffer, 3);
    if (result != 0)
    {
        printf("TMP006 ");
        displayI2cError(result);
    }
}

/** 
Reads the 16 bit value of a single 8 bit register
@pre I2C bus was initialized with i2cInit()
@param commandRegister which register to read
@return the value read from the register
@note The last byte read will be terminated with a NAK. This is required. 
*/
static uint16_t i2cRead16(uint8_t commandRegister)
{
    uint8_t result;
    unsigned char I2CBuffer[2];  //First byte is cmd reg, next two bytes are for address high/low

    I2CBuffer[0] = commandRegister;
    result = i2cWrite(I2CBuffer, 1);
    if (result != 0)
    {
        printf("TMP006 ");
        displayI2cError(result);
    }

    result = i2cRead(I2CBuffer, 2);          // Read from I2C line
    if (result != 0)
    {
        printf("TMP006 ");
        displayI2cError(result);
    }
    
    uint8_t msb = I2CBuffer[0];      
    uint8_t lsb = I2CBuffer[1];
#ifdef TCS3414_VERBOSE
    printf("Read %04X from register %02X\r\n", CONVERT_TO_INT(lsb, msb), commandRegister);
#endif
    return (CONVERT_TO_INT(lsb, msb));
}

/** 
Calculate temperature of an object based on tdie and vobj
@param tDie temperature of the die converted to Kelvin
@param vObj object voltage converted first by multiplying 1.5625e-7
@return temperature of an object in Celsius
@see TMP006 datasheet and application note
@note from TI TMP006 BoosterPack sample code
@note this may cause a stack overflow condition! Mind your stack size! In CCS the stack size is
configured in Project Properties : Build : ARM Linker : Basic Options. On an MSP430 this can cause
stack usage of ~180B; we configure stack size for 200B or more. On Stellaris we use 1024B. Obviously
these settings are much larger than the default.
*/
static float tmp006CalculateTemperature(const float * tDie, const float * vObj)
{    
#define S0      6.0E-14F            // Default S0 cal value
#define a1      1.75E-3F
#define a2      -1.678E-5F
#define b0      -2.94E-5F    
#define b1      -5.7E-7F
#define b2      4.63E-9F
#define c2      13.4f
#define Tref    298.15
    float S = S0*(1+a1*(*tDie - Tref)+a2*pow((*tDie - Tref),2));
    float Vos = b0 + b1*(*tDie - Tref) + b2*pow((*tDie - Tref),2);
    float fObj = (*vObj - Vos) + c2*pow((*vObj - Vos),2);
    float Tobj = pow(pow(*tDie,4) + (fObj/S),.25);      
    
    return (Tobj - 273.15);
}

//
//  Public Methods
//

/** 
Configures the TMP006 Sensor over I2C.
@pre I2C interface on microcontroller was configured correctly
@note Part maximum baud rate 400kHz for I2C.
@note On powerup, the sensor is powered down and needs to be turned on before use.
@note Manufacturer register should be 0x5449, and control register should be 0x0067
*/
void tmp006Init()   
{
    i2cInit(TMP006_I2C_ADDRESS);  
    printf("TMP006 Initialize\r\n");
    i2cWrite16(TMP006_P_WRITE_REG, TMP006_RST);  //reset the device - outputs on I2C: 0x80 (I2C address 0x40 shifted left 1, write), 0x02 (register), 0x80 (MSByte), 0x00 (LSB)
    i2cWrite16(TMP006_P_WRITE_REG, TMP006_POWER_UP + TMP006_CR_1); //I2C output: 0x80, 0x02 (register), 0x74 (MSB), 0x00 (LSB)

    // Optional - display contents of registers
    printf("    Control Register = 0x%04X\r\n", i2cRead16(TMP006_P_DEVICE_ID)); //I2C output: 0x80(write to 0x40), 0xFF (deviceID); 0x81 (read from 0x40), 0x00 (MSB), 0x67 (LSB)
    printf("    Manufacturer Register = 0x%04X\r\n", i2cRead16(TMP006_P_MAN_ID)); //I2C output: 0x80 (write to 0x40), 0xFE (manf ID); 0x81 (read from 0x40), 0x54, 0x49
}

/** 
Function to calulate and return the temperature and raw values from TMP006 sensor with transient
correction. Also initializes the I2C bus with the TMP006's slave address.
@pre I2C bus has been properly initialized
@return TempReading structure containing the temperature values read and converted from the TMP006.
*/
void tmp006GetTemperature(struct TempReading* tempRead)
{
    float vObjcorr = 0;
    float tslope = 0;
#define alpha      2.96E-4F
    
    /* Shift oldest tdie temp out */
    tDie[0] = tDie[1];
    tDie[1] = tDie[2];
    tDie[2] = tDie[3];
    
    /* Now, initialize the sensor. Required since we're sharing the bus with other peripherals */
    i2cInit(TMP006_I2C_ADDRESS);  
    
    /* Read the object voltage. Assuming that the data is ready. */
    tempRead->vObj = i2cRead16(TMP006_P_VOBJ);
    
    /* Read the ambient temperature */
    tempRead->tDie = i2cRead16(TMP006_P_TABT);

#ifdef TCS3414_VERBOSE
    printf("vObj=%d, tDie=%d\r\n", tempRead->vObj, tempRead->tDie);
#endif

    /* NOTE: This next section is for averaging values over a window of 4 readings */
    
    /* Convert latest tDie measurement to Kelvin */
    tDie[3] = (((float)(tempRead->tDie >> 2)) * .03125f) + 273.15f;
    
    if(tDie[0] != 0)	/* Executes only if all 4 tdie variables are non-zero */
    {
        /* Calculate tslope */
        tslope = -(0.3f*tDie[0]) - (0.1f*tDie[1]) + (0.1f*tDie[2]) + (0.3f*tDie[3]);
        
        /* Correct sensor voltage */
        vObjcorr = (((float)(tempRead->vObj)) * .00000015625f) + (tslope * alpha);
        
    } else {  /* Executes if all 4 tdie variables have not yet been filled - possibly erroneous info.*/
        vObjcorr = ((float)(tempRead->vObj)) * .00000015625f;
    }
    
    tempRead->temp = tmp006CalculateTemperature(&tDie[3], &vObjcorr);
    
    tempRead->tempInt = (int16_t) (100.0f * tempRead->temp);
}

/* @} */
