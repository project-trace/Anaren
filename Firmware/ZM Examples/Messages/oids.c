/**
*
* @file oids.c
*
* @brief Utility methods for object identifiers (oids)
*
* Of course you will probably want to change these for your own use. If you are implementing a
* sensor-to-server system then be sure that these match the OIDS table in your server.
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

#include "oids.h"
#include "../../HAL/hal.h"

/** 
Gets the name of the OID.
@param oid the Object Identifier to look up.
@return The name of the Object Identifier (oid) in human readable format, or "UNKNOWN" if not known.
*/
char* getOidName(uint8_t oid)
{
    switch (oid)
    {
    case OID_SUPPLY_VOLTAGE_MV:
        return "OID_SUPPLY_VOLTAGE_MV";
    case OID_TEMPERATURE_LOCAL:
        return "OID_TEMPERATURE_LOCAL";
    case OID_TEMPERATURE_IR:
        return "OID_TEMPERATURE_IR";
    case OID_LIGHT_SENSOR_AMBIENT_LUX:
        return "OID_LIGHT_SENSOR_AMBIENT_LUX";
    case OID_COLOR_SENSOR_RED:
        return "OID_COLOR_SENSOR_RED";
    case OID_COLOR_SENSOR_BLUE:
        return "OID_COLOR_SENSOR_BLUE";
    case OID_COLOR_SENSOR_GREEN:
        return "OID_COLOR_SENSOR_GREEN";
    case OID_COLOR_SENSOR_CLEAR:
        return "OID_COLOR_SENSOR_CLEAR";

    default:
        return "UNKNOWN";
    }
}

/**
Gets the name of the OID.
@param oid the Object Identifier to look up.
@return The name of the Object Identifier (oid) in human readable format, or "UNKNOWN" if not known.
*/
void displayFormattedOidValue(uint8_t oid, int16_t value)
{
    switch (oid)
    {
    case OID_SUPPLY_VOLTAGE_MV:
        printf("(%d.%03dV)", value/1000, value%1000);
        return;
    case OID_TEMPERATURE_LOCAL:
        printf("(%d.%02dC)", value/100, value%100);
        return;
    case OID_TEMPERATURE_IR:
        printf("(%d.%02dC)", value/100, value%100);
        return;
    case OID_LIGHT_SENSOR_AMBIENT_LUX:
        printf("(%dLux)", value);
        return;

    default:        // Don't display anything
        return;
    }
}
