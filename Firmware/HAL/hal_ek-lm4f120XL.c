/**
* @file hal_ek-lm4f120XL.c
*
* @brief Hardware Abstraction Layer (HAL) for the Stellaris LaunchPad EK-LM4F120XL. Based on hal_gw1.c
*
* Tools to interact with the onboard peripherals for Module interface. The EK-LM4F120XL is a
* development board utilizing a LM4F120XL processor and a few peripherals.
*
* This file must be modified if changing hardware platforms.
*
* The Zigbee Module library & examples require several methods to be defined.
* See hal_helper documentation. Also see hal_launchpad.h for macros that must be defined.
*
* @see hal_helper.c for utilities to assist when changing hardware platforms
*
* $Rev: 1568 $
* $Author: dsmith $
* $Date: 2012-09-24 17:16:29 -0700 (Mon, 24 Sep 2012) $
*
* @section support Support
* Please refer to the wiki at http://teslacontrols.com/mw/ for more information. Additional support
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

#include "hal.h"
#include "hal_version.h"
#include "../HAL/hal_ek-lm4f120XL_rgb.h"	// For RGB LED PWM
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/fpu.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "inc/hw_ssi.h"
#include "utils/uartstdio.h"
#include <stdint.h>


/** This is a function pointer for the Interrupt Service Routine called when a debug console
character is received. To use it, declare it with
<code> extern void (*debugConsoleIsr)(int8_t);  </code>
and then point it to a function you created, e.g.
<code> debugConsoleIsr = &handleDebugConsoleInterrupt;  </code>
and your function handleDebugConsoleInterrupt() will be called when a byte is received on the debug
serial port.
Obviously your handleDebugConsoleInterrupt must have the same method signature as the
function pointer. In other words, your function should be declared as:
void handleDebugConsoleInterrupt(int8_t);
*/
void (*debugConsoleIsr)(int8_t);

/** Function pointer for the ISR called when the button is pressed.
Parameter is which button was pressed. */
void (*buttonIsr)(int8_t);

/** Initializes Oscillator: configures MCLK to 25MHz using internal PLL (SYSCTL_USE_PLL) calibrated
 * to 8MHz external crystal (SYSCTL_XTAL_8MHZ).
 * @note PLL native frequency = 200MHz, so divide by 8 (SYSCTL_SYSDIV_8) to get 25MHz
 * @note ADC requires either the PLL or a 16MHz clock source
 */
void oscInit()
{
    SysCtlClockSet(SYSCTL_SYSDIV_8 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
}

#define PERIPHERAL_ENABLE_DELAY()	    SysCtlDelay(10) //delay by (3 * 10) 30 clock cycles

#define LEFT_BUTTON             GPIO_PIN_4
#define RIGHT_BUTTON            GPIO_PIN_0
#define ALL_BUTTONS             (LEFT_BUTTON | RIGHT_BUTTON)

/** Initializes Ports/Pins: sets direction, interrupts, pullup/pulldown resistors etc. */
void portInit()
{
    /* Port A
     * PA0      U0Rx (Debug UART)
     * PA1      U0Tx (Debug UART)
     * PA2      Bit-Bang I2C SDA
     * PA3      Bit-Bang I2C SCL
     * PA4      BoosterPack RGB LED - Green
     * PA5      Module SS & MRDY
     * PA6		BoosterPack RGB LED - Red
     * PA7		Module SRDY
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    PERIPHERAL_ENABLE_DELAY();
    /* Configure UART pins */
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    /* Outputs */
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_6, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); // For LEDs
    /* Inputs */
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);
    /* Bit-bang I2C */
    //GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_OUT);  //SDA & SCL
    //GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_OD);  //SDA & SCL open-drain
    //GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_2 | GPIO_PIN_3);  //Set SDA & SCL high

    /* softI2c */
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    /* Port B
     * PB0
     * PB1
     * PB2		BoosterPack RGB LED - Blue
     * PB3
     * PB4		Module SCLK		SSI2CLK
     * PB5      LED0
     * PB6      Module MISO		SSI2Rx
     * PB7		Module MOSI		SSI2Tx
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  //FOR STATUS LED
    PERIPHERAL_ENABLE_DELAY();
    /* SSI2 Configuration for Module SPI */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    PERIPHERAL_ENABLE_DELAY();
    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);
    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
    /* Outputs */
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_5, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);

    /* Port C
     * PC0		(JTAG TCK/SWCLK)
     * PC1		(JTAG TMS/SWDIO)
     * PC2		(JTAG TDI)
     * PC3		(JTAG TDIO/SWO)
     * PC4
     * PC5
     * PC6
     * PC7
     */
    // Note: no initialization needed for Port C required; using default configuration

    /* Port D
     * PD0		I2C3SCL
     * PD1		I2C3SDA
     * PD2
     * PD3
     * PD4		(USB D-)
     * PD5      (USB D+)
     * PD6
     * PD7		(USB VBUS)
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);  //FOR STATUS LED
    PERIPHERAL_ENABLE_DELAY();
    /* I2C Configuration */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C3); //requires 5 clock cycles to initialize
    PERIPHERAL_ENABLE_DELAY();
    GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0); //NOTE: Only required for blizzard (LM4F) parts
    GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD0_I2C3SCL);
    GPIOPinConfigure(GPIO_PD1_I2C3SDA);
    I2CMasterInitExpClk(I2C3_MASTER_BASE, SysCtlClockGet(), false); //FALSE = 100kbps
    SysCtlDelay(10000); //otherwise portion of SlaveAddrSet() lost - only for blizzard

    /* Port E - note this port is only 6 pins
     * PE0		Module Reset
     * PE1
     * PE2
     * PE3
     * PE4		Switch S2 on BoosterPack - note has external 47k pullup on BoosterPack
     * PE5      Current Sensor analog input
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    PERIPHERAL_ENABLE_DELAY();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    PERIPHERAL_ENABLE_DELAY();
    /* Outputs */
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);
    /* Inputs */
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4 , GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
    GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
    GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE); // Make button a falling-edge triggered interrupt
    GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_4);					// Enable the interrupt on this pin
    GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_4);					//Clear interrupts
    IntEnable(INT_GPIOE);//enable interrupt 18

    /* ADC Inputs */
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    RADIO_OFF();
    SPI_SS_CLEAR();

    /* Port F
     * PF0      Switch SW2 on Stellaris LaunchPad
     * PF1		RGB LED on Stellaris LaunchPad - Red
     * PF2		RGB LED on Stellaris LaunchPad - Green
     * PF3		RGB LED on Stellaris LaunchPad - Blue
     * PF4		Switch SW1 on Stellaris LaunchPad
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    PERIPHERAL_ENABLE_DELAY();
    /* Outputs */
    // Note: PWM Outputs for LEDs are initialized separately if they are being used
    /* Inputs */
    // Unlock PF0 so we can change it to a GPIO input. see ButtonsInit() in buttons.c in example qs-rgb
    // Once we have enabled (unlocked) the commit register then re-lock it
    // to prevent further changes.  PF0 is muxed with NMI thus a special case.
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    GPIODirModeSet(GPIO_PORTF_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTF_BASE, ALL_BUTTONS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTF_BASE, ALL_BUTTONS, GPIO_FALLING_EDGE); // Make button a falling-edge triggered interrupt
    GPIOPinIntEnable(GPIO_PORTF_BASE, ALL_BUTTONS);                  // Enable the interrupt on this pin
    GPIOPinIntClear(GPIO_PORTF_BASE, ALL_BUTTONS);                   //Clear interrupts
    IntEnable(INT_GPIOF);
}

/** Initialize UART0 to use for stdio
 * @pre UART0 pins were configured
 */
void halUartInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    PERIPHERAL_ENABLE_DELAY();
    UARTStdioInit(0);  //configures this UART0 as the uart to use for UARTprintf
    IntEnable(INT_UART0);       // If interrupts are desired, then enable interrupts on this UART
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

/** Simple placeholder function to point the function pointers to so that they don't cause mischief */
void doNothing(int8_t a)
{
    PERIPHERAL_ENABLE_DELAY();  //prevent this from getting optimized out
}

/** Display information about this driver firmware */
void displayVersion()
{
    int i = 0;
    printf("\r\n");
    for (i=0; i<8; i++)
        printf("-");
    printf(" Module Interface and Examples %s ", MODULE_INTERFACE_STRING);
    for (i=0; i<8; i++)
            printf("-");
    printf("\r\n");
    printf("%s", MODULE_VERSION_STRING);
}

/**
Configures hardware for the particular hardware platform:
- Ports: sets direction, interrupts, pullup/pulldown resistors etc.
- Holds radio in reset (active-low)
*/
void halInit()
{
    //
    // Enable the floating-point unit.
    //
    FPUEnable();
    //
    // Configure the floating-point unit to perform lazy stacking of the
    // floating-point state.
    //
    FPULazyStackingEnable();

	oscInit();
	portInit();
	halUartInit();

    //Point the function pointers to doNothing() so that they don't trigger a restart
    debugConsoleIsr = &doNothing;
    buttonIsr = &doNothing;
    clearLeds();
    displayVersion();
}

/** Port E interrupt service routine
@note Must be configured in startup_ccs.c or else will not be called.
*/
void IntGPIOe(void)
{
    buttonIsr(0);   // Button 0 was pressed
    GPIOPinIntClear(GPIO_PORTE_BASE, GPIO_PIN_4);					//Clear interrupts
}

/** Port F interrupt service routine
@note Must be configured in startup_ccs.c or else will not be called.
*/
void IntGPIOf(void)
{
    uint32_t buttonState = GPIOPinRead(GPIO_PORTF_BASE, ALL_BUTTONS);
    if ((~buttonState) & LEFT_BUTTON)
        buttonIsr(1);
    else if ((~buttonState) & RIGHT_BUTTON)
        buttonIsr(2);
    GPIOPinIntClear(GPIO_PORTF_BASE, ALL_BUTTONS);                   //Clear interrupts
}

/*
Whether the selected button is pressed.
@param button which button - must be ANY_BUTTON or BUTTON_1 or BUTTON_2 on this implementation.
@return > 0 if the selected button is pressed, otherwise 0.
 */
uint8_t buttonIsPressed(uint8_t button)
{

    uint32_t buttonState = GPIOPinRead(GPIO_PORTF_BASE, ALL_BUTTONS);
    uint32_t buttonState2 = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4);

    if (button == ANY_BUTTON)
        return (((~buttonState) & LEFT_BUTTON) || ((~buttonState) & RIGHT_BUTTON) || ((~buttonState2) & GPIO_PIN_4));

    if (button == BUTTON_0)
        return ((~buttonState) & LEFT_BUTTON);
    if (button == BUTTON_1)
        return ((~buttonState) & RIGHT_BUTTON);

    if (button == BUTTON_2)
        return ((~buttonState2) & GPIO_PIN_4);

    // invalid buttonId, so return false
    return 0;
}


/** The UART has a 16 byte FIFO buffer, so our printf() methods return even though the processor
 * is still outputting bytes on the UART.
 * @return 1 if the Debug UART is busy sending data or 0 if not. Useful to postpone sleeping until the UART
 * has sent all data.
 */
uint8_t halUartBusy()
{
	return (UARTBusy(UART0_BASE));
}


/**
 Debug console interrupt service routine, called when a byte is received on UART.
 @pre In startup_ccs.c, UARTIntHandler is set as the ISR for the UART0 Rx and Tx interrupt
 @pre In startup_ccs.c, this function is declared, e.g:
 <pre>
 extern void UARTIntHandler(void);
 </pre>
 @pre Interrupts have been enabled, e.g:
 <pre>
 IntMasterEnable();
 IntEnable(INT_UART0);
 UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
 </pre>
 @pre UART has been configured
 @post a byte received on the UART will result in debugConsoleIsr() function pointer called
 */
void UARTIntHandler(void)
{
    unsigned long ulStatus;

    // Get the interrupt status.
    ulStatus = UARTIntStatus(UART0_BASE, true);

    // Clear the asserted interrupts.
    UARTIntClear(UART0_BASE, ulStatus);

    // Loop while there are characters in the receive FIFO.
    while(UARTCharsAvail(UART0_BASE))
    {
        // Read the next character from the UART
    	uint8_t c = (uint8_t) UARTCharGetNonBlocking(UART0_BASE);
        debugConsoleIsr(c);    // call the function pointer
    }
}

#define NUM_LEDS 4

/**
Toggles the specified LED.
@param led the LED to toggle
@post The specified LED is toggled.
@return 0 if success, -1 if invalid LED specified
*/
int16_t toggleLed(uint8_t led)
{
	if (led >= NUM_LEDS)
		return -1;
	switch (led)
	{
	case 0:
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5,~GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5));
		break;
	case 1:		// Red
		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,~GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_6));
		break;
	case 2:
		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,~GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4));
		break;
	case 3:
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2,~GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2));
		break;
	default:		// should never happen
		return -2;
	}

	return 0;
}

/**
Turns ON the specified LED.
@param led the LED to turn on
@post The specified LED is turned on.
@return 0 if success, -1 if invalid LED specified
*/
int16_t setLed(uint8_t led)
{
	if (led >= NUM_LEDS)
		return -1;
	switch (led)
	{
	case 0:
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
		break;
	case 1:		// Red
		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);
		break;
	case 2:     // Green
		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0);
		break;
	case 3:     // Blue
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
		break;
	default:		// should never happen
		return -2;
	}
	return 0;
}

/**
Turns OFF the specified LED.
@param led the LED to turn off
@post The specified LED is turned on.
@return 0 if success, -1 if invalid LED specified
*/
int16_t clearLed(uint8_t led)
{
    if (led >= NUM_LEDS)
        return -1;
    switch (led)
    {
    case 0:
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
        break;
    case 1:     // BoosterPack RGB Red
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);
        break;
    case 2:     // BoosterPack RGB Green
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);
        break;
    case 3:     // BoosterPack RGB Blue
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
        break;
    default:        // should never happen
        return -2;
    }
    return 0;
}

/**
Turns OFF LEDs.
@post LEDs are turned off.
*/
void clearLeds()
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);  //turn off LED
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
}

/**
Initializes the Serial Peripheral Interface (SPI) interface to the Zigbee Module (ZM).
@note Maximum module SPI clock speed is 4MHz. SPI port configured for clock polarity of 0, clock
phase of 0, and MSB first.
@note The Stellaris LaunchPad uses SSI2 to communicate with module
@pre SPI pins configured correctly:
- Clock, MOSI, MISO configured as SPI function
- Chip Select configured as an output
- SRDY configured as an input.
@post SPI port is configured for communications with the module.
*/
void halSpiInitModule()
{
    // Disable the SSI Port
    //SSIDisable(SSI2_BASE);

    // Reconfigure the SSI Port for Module operation.
    // Clock polarity = inactive is LOW (CPOL=0); Clock Phase = 0; MSB first; Master Mode, 2MHz, data is 8bits wide;
    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 8);

    // Enable the SSI Port
    SSIEnable(SSI2_BASE);

    //
    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SPI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    //
    unsigned long ulDataRx[5];
    while(SSIDataGetNonBlocking(SSI2_BASE, &ulDataRx[0]))
    {
    }

    // Don't select the module
    SPI_SS_CLEAR();
}

/**
Sends a message over SPI to the Module.
SPI uses a "write-to-read" approach to read data out, you must write data in.
This is a private method that gets wrapped by other methods, e.g. spiSreq(), spiPoll(), etc.
To Write, set *bytes, numBytes.
To Read, set *bytes only. Don't need to set numBytes because Module will stop when no more bytes read.
@param bytes the data to be sent or received.
@param numBytes the number of bytes to be sent. This same buffer will be overwritten with the received data.
@note Modify this method for other hardware implementations.
@pre SPI port configured for writing
@pre Module has been initialized
@post bytes contains received data, if any
@see Stellaris Application Note spma002.pdf, "Adding 32kB of Serial SRAM to a Stellaris Microcontroller"
*/
void spiWrite(unsigned char *bytes, unsigned char numBytes)
{
    unsigned long ulReadData;
	while(numBytes--)    // Loop while there are more bytes left to be transferred.
    {
        SSIDataPut(SSI2_BASE, *bytes);              // Write the next byte to the SSI controller with a blocking put.
        SSIDataGet(SSI2_BASE, &ulReadData);         // Read into a long first
        *bytes++ = (unsigned char)ulReadData;       // ...and then convert it to a char
    }

	/* Note: Stellaris processors have a tx/rx FIFO. If you'd like to wait until the FIFO is empty
	 * before returning from this method, include the following code:
	 */
    //while ( (~HWREG(SSI0_BASE + SSI_O_SR)) & SSI_SR_TFE) ;   //wait while Transmit FIFO is NOT empty
	//while ( (HWREG(SSI0_BASE + SSI_O_SR)) & SSI_SR_BSY) ;   //wait while busy - works without this
}


/**
A fairly accurate Blocking Delay in Milliseconds - delays by at least the specified number of
milliseconds (mSec)
@pre SysCtlClockSet() has been called to set the processor clock rate
@param ms number of milliseconds to delay
@note change if you are using an RTOS.
*/
void delayMs(uint16_t ms)
{
	SysCtlDelay((SysCtlClockGet() / 3000) * ms);   // Is 1000 * 3 (since each SysCtlClockGet() call is 3 clock cycles)
}

/**
Initializes the PWM engine used for the RGB LED. This allows the RGB LED to display many colors.
@post RGB LED may be used, with halRgbSetLeds().
*/
void halRgbLedPwmInit()
{
    RGBInit(0);
	RGBIntensitySet(0.5f);
    RGBEnable();
    halRgbSetLeds(0x7F, 0x7F, 0x7F);   // A dim white
}

/**
Sets RGB LED color to the selected values.
@pre halRgbLedPwmInit() has been called to initialize the PWM engine.
@post RGB LED displays the selected colors.
@note you must include hal_ek-lm4f120XL_rgb.h in this file and ensure that the .c file is included in the build path
*/
void halRgbSetLeds(uint8_t red, uint8_t blue, uint8_t green)
{
    unsigned long ulColor[3];
#ifdef DEBUG_HAL_RGB_SET_LEDS
    printf("halRgbSetLeds: R=%02x, B=%02x, G=%02x\r\n", red, blue, green);
#endif
    ulColor[RED] = (unsigned long) ( (red << 8) * (RED_WHITE_BALANCE));
    ulColor[BLUE] = (unsigned long) ( (blue << 8) * (BLUE_WHITE_BALANCE));
    ulColor[GREEN] = (unsigned long) ( (green << 8) * (GREEN_WHITE_BALANCE));

    RGBColorSet(ulColor);
}


/** Required for compatibility with MSP430 libraries */
int16_t calibrateVlo()
{
	return 1;
}

//
//      Analog to Digital Converter
//

#include "driverlib/adc.h"

/**
Reads the current sense amplifier output using the Analog to Digital Converter (ADC).
Current sense resistor is 0.2 ohms, amplifier multiplies voltage drop by 50. Therefore, every mA of
current translates into 10mV measured by the microcontroller
@note Current sense input is on PE5 on Stellaris LaunchPad
@pre Current sense shunt is installed on the BoosterPack
@return Current in mA multiplied by 10 (e.g. return value of 713 = 71.3mA)
@see StellarisWare/examples/peripherals/adc/single_ended.c
*/
uint16_t getCurrentSensor()
{
    /* This array is used for storing the data read from the ADC FIFO. It must be as large as the
    FIFO for the sequencer in use.  This example uses sequence 3 which has a FIFO depth of 1. */
    unsigned long ulADC0_Value[1];

    /* Enable sample sequence 3 with a processor signal trigger.  Sequence 3 will do a single sample
    when the processor sends a signal to start the conversion. */
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    /* Configure step 0 on sequence 3.  Sample channel 8 (ADC_CTL_CH8) in single-ended mode
     * (default) and configure the interrupt flag (ADC_CTL_IE) to be set when the sample is done.
     * Tell the ADC logic that this is the last conversion on sequence 3 (ADC_CTL_END). */
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH8 | ADC_CTL_IE | ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    ADCIntClear(ADC0_BASE, 3);

    //
    // Now the setup is done, let's actually measure something!
    //

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC0_BASE, 3);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    // Clear the ADC interrupt flag.
    ADCIntClear(ADC0_BASE, 3);

    // Read ADC Value into ulADC0_Value
    ADCSequenceDataGet(ADC0_BASE, 3, ulADC0_Value);

    // This part does not have a separate voltage reference
    // Therefore we have to measure with reference to what we think VCC is, 3.30V
#define REFERENCE_VOLTAGE_MV                (3300l)

    uint32_t temp = (ulADC0_Value[0] * REFERENCE_VOLTAGE_MV);

#define NUMBER_OF_STEPS_12_BIT_RESOLUTION   (4096l)

    return ((uint16_t) (temp / NUMBER_OF_STEPS_12_BIT_RESOLUTION));
}

//
//		Timer
//

/** Function pointer for the ISR called when a timer generates an interrupt*/
void (*timerIsr)(void);

/** ISR for Timer 2
 * @pre timer configured with initTimer
 * @pre global interrupts enabled with HAL_ENABLE_INTERRUPTS()
 */
void Timer2IntHandler(void)
{
    // Clear the timer interrupt
    ROM_TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    // Call the timerIsr function pointer
    timerIsr();
}

/**
Configures timer.
@see timers example in EK-LM4F120XL StellarisWare package
@note On the LM4F120XL, there are 16/32 bit timers or "wide" timers featuring 32/64bit. We use a standard timer.
@param seconds period of the timer. Maximum is 0xFFFFFFFF / SysCtlClockGet(); or about 171 sec if using 25MHz main clock
@return 0 if success; -1 if illegal parameter
*/
int16_t initTimer(uint8_t seconds)
{
#define TIMER_MAX_SECONDS	(0xFFFFFFFF / SysCtlClockGet())

    if ((seconds > TIMER_MAX_SECONDS) || (seconds == 0))
        return -1;

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);		// Full width (32 bit for Timer 2) periodic timer
    ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, (ROM_SysCtlClockGet() * seconds));  // for once per second
    ROM_IntEnable(INT_TIMER2A);
    ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER2_BASE, TIMER_A);

    return 0;
}

/** Required for compatibility with MSP430 library */
void halSetWakeupFlags(uint16_t wakeupFlagsToSet)
{

}
