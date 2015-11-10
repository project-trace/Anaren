/**
@mainpage Zigbee Module Examples
@brief READ ME FIRST
@author Derek Smith

@section intro Introduction
The Zigbee Module allows you to easily create Zigbee applications without having to learn an intricate Zigbee stack. The module contains the Zigbee stack and interfacing is easy through the module library. This page is a general overview of the examples we've provided to interface with the Module. See documentation in each example .c file for more information. Methods are commented in the .c files. Header files list public (externally accessible) methods only.

@section license License

Copyright (c) 2012 Tesla Controls. All rights reserved. Redistribution and use in source and
binary forms, with or without modification, are permitted only if used solely and exclusively with
an Anaren A2530E24AZ1, A2530E24CZ1, A2530R24AZ1, or A2530R24CZ1 module. Refer to the file 
 "anaren_eula.txt" for additional restrictions and limitations on this Software.

 @section org File Organization
 The examples make use of the module interface library. This abstraction layer allows you to focus on your application without having to worry about the details of the interface. This library relies on lower levels of abstraction to interface with the hardware. There are four layers of firmware provided:
- Examples: these run on the host microcontroller to demonstrate module functionality. Almost 30 different examples are included; everything from basic communications with the module to a complete Zigbee sensor application.
- Module Driver: this is a lightweight wrapper library that runs on the host microcontroller and allows an application developer to code to an application programming interface (API) instead of worrying about bytes over the wire. This includes the files in the "ZM" directory like af.c, zdo.c, simple_api.c, module.c, etc. NOTE: this driver library is not required (all the complicated processing is handled inside the module) but makes development much easier.
- Physical Interface: This file (zm_phy_spi.c if using SPI, zm_phy_uart if using UART) acts as thin layer between the drivers and the hall file, allowing you to use either interface without much changes.
- Hardware Abstraction Layer (HAL): The HAL allows easy porting to different processors since only one file needs to be changed. We have developed HALs for the TI MSP430F2274, TI MSP430F248, and TI Stellaris processors. This also includes a utility to assist the user in porting to a different processor. - Module Firmware: this runs on the module and provides the Zigbee PRO interface and handles sending/receiving of data. It interfaces via SPI with a host processor and wraps the TI Z-Stack which has been used on millions of Zigbee devices.

Below is the hierarchy of files:
  <table border>
  <tr>
     <td colspan=6 align=center><b> User Example (e.g. Simple Communications Example) </b></td>
   </tr>

   <tr>
      <td colspan=2 align=center> Utilities </td>
      <td colspan=2 align=center> AF/ZDO Interface </td>
      <td colspan=2 align=center> SimpleAPI Interface </td>
   </tr>
   <tr>
      <td colspan=3 align=center> SPI PHY </td>
      <td colspan=3 align=center> UART PHY </td>
   </tr>
   <tr>
      <td colspan=6 align=center> Hardware Abstraction Layer (HAL) </td>
   </tr>
   <tr>
      <td colspan=6 align=center> <i>Low Level Hardware Registers / Module Firmware</i> </td>
   </tr>
</table> 

 @section Examples
The included examples demonstrate basic functionality to help the user get up and running quickly. There are four categories of examples: - Hardware Driver examples demonstrating basic I/O with the development board - Module Interface examples showing how to interface with the module - Communications examples showing many different ways to communicate data between two or more devices - A Sample Application showing how to use the module in a real application, with advanced fault-tolerance.

 @subsection hw Hardware Interface Examples
The \ref hwExamples "Hardware Interface Examples" contain basic hardware examples for the board. Start here, and compile, load, and run through each example. The recommended order is:
- example_button_interrupt.c
- example_hello_world.c
- example_timer_interrupt.c
- example_read_supply_voltage.c

If you would like to learn more about the peripherals on the BoosterPack, see:
- example_read_color_sensor.c
- example_read_eeprom.c
- example_read_IR_temperature_sensor.c

 @subsection iface Module Interface
The \ref moduleInterface "Module Interface Examples" contains basic hardware examples for the board. Once you have completed a few of the Hardware Interface Examples run through each of these examples. The recommended order is:
- example_reset_module.c
- example_get_mac_address.c
- example_get_version.c

If you would like to learn more about the module features, see:
- example_measure_module_current.c
- example_get_random.c
- example_read_nonvolatile_memory.c
- example_write_nonvolatile_memory.c
- example_read_digital_io.c
- example_write_digital_io.c
- example_rf_tester.c

 @subsection comms Communications
The \ref moduleCommunications "Basic Communications Examples" demonstrate how to communicate over Zigbee using the Module. For these examples you will need two devices, one to make the coordinator and one to make the other device (end-device or router). To get up and running, run the example_basic_comms_coordinator_afzdo.c on one device and then example_basic_comms_router_afzdo.c on a second device. Once you get these examples working, load a third device with example_basic_comms_end_device_afzdo.c and you can see how a basic network works. Additional examples include:
- example_basic_comms_coordinator_sapi.c
- example_basic_comms_router_sapi.c

For experimenting with the module's handling of large messages, see:
- example_fragmentation_coordinator_afzdo.c
- example_fragmentation_router_afzdo.c

Finally, to test the module's security features, see:
- example_secure_comms_coordinator_afzdo.c
- example_secure_comms_router_afzdo.c

 @subsection apps Simple Applications
The \ref apps "Simple Application Examples" demonstrates a simple sensor application. These are designed to help you get up and running quickly. Note: you should run and be familiar with the previous examples before using these. The recommended order is:
- example_simple_application_coordinator_afzdo.c
- example_simple_application_router_afzdo.c
- example_simple_application_end_device_afzdo.c

In this group is the example_network_explorer.c utility that uses a command line interface to send/receive messages.

 @section iface Module Interface
The Zigbee Module (ZM) allows you to easily create Zigbee applications with the included interface. There are two interfaces to the Module: the Simple API and the Application Framework / Zigbee Device Objects (AFZDO). Simple API is a little easier, as it is only ~10 methods to learn, but AFZDO is more powerful. Most examples are written using the AFZDO interface.

This directory includes the drivers to the module, including:

 @subsection sys System Interface
 These are the generic Module configuration methods and can be used with Simple API or AFZDO.
- module_commands.h: contains all the commands used.
- module.c: This is the bulk of the interface, for configuring and using the module.

 @subsection api Application Programming Interface
 This enables your application to send/receive data using the Module. 
 Use either Simple API or AFZDO. The are compatible, but once you've started your application with Simple API then you should only use SimpleAPI commands to send receive data.
- simple_api.c: Implements the Simple API
- af.c: Implements the Application Framework interface
- zdo.c: Implements the Zigbee Device Objects interface
- application_configuration.c: used by both interfaces for advanced configuration of module behavior

@subsection zmhw Zigbee Module Hardware Interface
 This has been separated out from the main files to allow you to switch between SPI or UART whilst keeping the rest of the files the same.
- zm_phy.h: This will load the appropriate physical interface file based on the compile options.
- zm_phy_spi.c: SPI interface
- zm_phy_uart.c: UART interface

@subsection utils Miscellaneous Zigbee Module Utilities
Methods or defines used by one or more of the previous files.
- module_errors.h: describes the error handling system and how to use it.
- module_errors.c: defines the error codes used, and a simple utility for checking the return status.
- module_utilities.c: various utilities used for working with the module

 @section changing Changing Hardware Platforms
The Hardware Abstraction Layer (HAL) provides an easy way to port the Zigbee Module library and 
examples to other hardware environments.

Prior to changing hardware platforms, it is recommended:
 - You have already compiled and ran all the module examples on one of the supported platforms (MSP430 Launchpad, Stellaris LaunchPad, etc.) and you are familiar with how the library is structured.
 - You have extensive programming experience with your desired hardware environment
 - You have sufficient time. Porting HAL to a new processor (e.g. Stellaris ARM) took approximately 3 days

To change hardware platforms, the following steps are recommended:
 - Copy the hal files (e.g. hal_launchpad.c and hal_launchpad.h) that you wish to use as a starting point and save them with the name of your platform, for example "hal_gw0.c".
 - Define a project property with the name of your environment (e.g. GW0)
 - In hal.h add an option for your environment
 - Modify your new hal file for your hardware platform.

@see hal_helper.c for methods to assist in changing hardware platforms.


@note Projects compiled with IAR Embedded Workbench for IAR version 5.30

\defgroup hwExamples Hardware Driver Examples
\defgroup moduleInterface Module Interface Examples
\defgroup moduleCommunications Communications Examples
\defgroup apps Simple Applications


 <hr>
@page about About this Documentation
This HTML output generated by Doxygen, a free documentation generator tool 
that uses special tags in the code to generate pretty HTML output that you see here. 
It is very similar to javadoc if you have experience with that, in fact many of the tags are the same. 
Documentation generators like this are highly recommended, especially if you're writing code that someone else will need to read.
@see www.doxygen.org

*/