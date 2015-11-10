/**
*
* @file header.c
*
* @brief Common message header
*
* This is a message header that has the basics:
* - MAC Address
* - Protocol Version
* - Flags
* - Sequence Number
*
* Of course you will probably want to change this header format for your own use, but this format
 * works fairly well for most projects. The MAC Address is included to make it easier when the
 * message is converted into an Ethernet packet; the Protocol Version allows you to change protocols
 * in the future; the flags are for generic use; and the Sequence Number is handy when you have lots
 * of sequential data and you need to re-order it on the server side.
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

#include "header.h"
#include "../../HAL/hal.h"
#include "../../Common/utilities.h"
#include <string.h>

/** uncomment to display more information */
//#define VERBOSE_HEADER

/**
Displays the contents of this header in a human readable format, for example:
Info #317: V/F=02/00, mac=00124B00015FB38E
@param h points to the header that will be displayed
*/
void printHeader(struct header* h)
{
  printf("#%u: V/F=%02X/%02X, mac=", h->sequence, h->version, h->flags);
  int i=7;
  while (i--)
    printf("%02X", h->mac[i]);
}

/**
Serializes the Header into memory starting at destination.
Number of bytes streamed is equal to getSizeOfHeader() 
@param h points to the header to serialize
@pre destination points to a region of memory at least getSizeOfHeader() bytes in size
*/
void serializeHeader(struct header* h, unsigned char* destination)
{
  unsigned char* destinationPtr = destination;
  *destinationPtr++ = h->sequence & 0xFF;  //LSB first
  *destinationPtr++ = h->sequence >> 8;
  *destinationPtr++ = h->version; 
  *destinationPtr++ = h->flags;
  memcpy(destinationPtr, h->mac, 8);
}

/**
Deserializes the stream of bytes into a struct header
@param source points to a sequence of bytes that is a serialized header, getSizeOfHeader() bytes long
@return a header structure created from the bytes
*/
struct header deserializeHeader(unsigned char* source)
{
  unsigned char* sourcePtr = source;
  struct header h;
  unsigned char lsb = *sourcePtr++;
  unsigned char msb = *sourcePtr++;
  h.sequence = lsb + 0x0100 * msb;
  h.version = *sourcePtr++;  //version
  h.flags = *sourcePtr++;  //flags
  memcpy(h.mac, sourcePtr, 8);
#ifdef VERBOSE_HEADER  
  printHeader(h);
  printf("\r\n");
  printHexBytes(source, 12);
  printf("\r\nHeader Parsed %u bytes\r\n", sourcePtr-source);
#endif  
  return h;
}
