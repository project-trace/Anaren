/**
*
* @file infoMessage.c
*
* @brief message containing information about this device. This is commonly used to send sensor data.
* 
* Info Message Cluster = 0x0007
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


#include "infoMessage.h"
#include "../../HAL/hal.h"
#include "../../Common/utilities.h"

/** 
Display the contents of the infoMessage to console 
@param im the Info Message to display
*/
void printInfoMessage(struct infoMessage* im)
{
    printf("Info ");
    printHeader(&(im->header));
    printf(", Device Type=%02X, numParameters=%u: ", im->deviceType, im->numParameters);
    int parameter = 0;
    for (parameter=0; parameter < im->numParameters; parameter++)
    {
        printf("<%02X=%04X> ", im->kvps[parameter].oid, im->kvps[parameter].value);
    }
    printf("\r\n");
}

/** 
Convenience method for adding a KVP to the infoMessage.
@param im the infoMessage to add a kvp to
@param kvp points to the kvp to add. This will be deep copied into the infoMessage.
@return 0 if success, -1 if there are too many kvps
*/
int8_t addKvpToInfoMessage(struct infoMessage* im, struct kvp *kvp)
{
    if (im->numParameters > MAX_KVPS_IN_STATUS_MESSAGE)
        return -1;
    im->kvps[im->numParameters].oid = kvp->oid;
    im->kvps[im->numParameters].value = kvp->value;    
    im->numParameters++;
    return 0;
}

/** 
Converts the info message into a stream of bytes to memory pointed to by destinationPtr
Number of bytes streamed is equal to getSizeOfInfoMessage() 
@param im the infoMessage to serialize
@param destinationPtr where to copy the stream of bytes to
@pre destinationPtr points to a region of memory at least getSizeOfInfoMessage() bytes in size
*/
void serializeInfoMessage(struct infoMessage* im, unsigned char* destinationPtr)
{  
  serializeHeader((&(im->header)), destinationPtr);
  destinationPtr += (HEADER_SIZE);
  *destinationPtr++ = im->deviceType;
  *destinationPtr++ = im->numParameters;
  
  int i;
    for (i=0; i < im->numParameters; i++)  //for each KVP:
  {
    *destinationPtr++ = im->kvps[i].oid;            //object identifier
    *destinationPtr++ = (im->kvps[i].value) & 0xFF;   //LSB of value
    *destinationPtr++ = (im->kvps[i].value) >> 8;          //MSB of value
  }
}

/** 
Creates an infoMessage from the stream of bytes starting at source
@param source the beginning of the serialized Info Message
@param info the infoMessage to create
@return the number of bytes deserialized if success, or else an error code < 0
*/
int16_t deserializeInfoMessage(unsigned char* source, struct infoMessage* info)
{
    unsigned char* sourcePtr = source;    
    info->header = deserializeHeader(sourcePtr);    // First, deserialize the header
    sourcePtr += (HEADER_SIZE);
    info->deviceType = *sourcePtr++;                // Get the deviceType
    info->numParameters = *sourcePtr++;             // ... and the number of Parameters
    if (info->numParameters > MAX_KVPS_IN_STATUS_MESSAGE)
        return -1;
    int i;
    for (i=0; i < info->numParameters; i++)  //for each Parameter:
    {
        info->kvps[i].oid = *sourcePtr++;
        info->kvps[i].value = CONVERT_TO_INT( (*sourcePtr), (*(sourcePtr+1)) );
        sourcePtr += 2;
    }
    return (sourcePtr - source);
}


/** Gets the size of this info message.
@param im the infoMessage to get the size of
@return the size of the Info Message, including all KVPs.
This is implemented as a method (not macro) for consistency with other messages.
*/
uint16_t getSizeOfInfoMessage(struct infoMessage* im)
{ 
  return (HEADER_SIZE) + 2 + ((im->numParameters) * SIZE_OF_KVP_IN_BYTES);
}

