//*****************************************************************************
//
// project0.c - Example to demonstrate minimal TivaWare setup
//
// Copyright (c) 2012-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"

//*****************************************************************************
//
// Define pin to LED color mapping.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Project Zero (project0)</h1>
//!
//! This example demonstrates the use of TivaWare to setup the clocks and
//! toggle GPIO pins to make the LED's blink. This is a good place to start
//! understanding your launchpad and the tools that can be used to program it.
//! See http://www.ti.com/tm4c123g-launchpad/project0 for more information and
//! tutorial videos.
//
//*****************************************************************************

#define ADDRESS_MATCH   GPIO_PIN_2	//PB2
#define DATA_READY  	GPIO_PIN_6	//PB6
#define CARRIER_DETECT	GPIO_PIN_7	//PB7
#define TRX_CE			GPIO_PIN_0	//PB0
#define PWR_UP			GPIO_PIN_1	//PB1
#define TX_EN			GPIO_PIN_5	//PB5

#define SSI0_FREC		1e6		//1[MHz]
#define	SSI0_DATAW		8

#define ON				0xFF
#define OFF				0x00

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Main 'C' Language entry point.  Toggle an LED using TivaWare.
// See http://www.ti.com/tm4c123g-launchpad/project0 for more information and
// tutorial videos.
//
//*****************************************************************************
int main(void)  {
    char Mensaje[32];
    uint8_t i;
    uint32_t Aux;

	//
    // Setup the system clock to run at 50 Mhz from PLL with crystal reference
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                    SYSCTL_OSC_MAIN);

    //--------------------------------------------------------------------------------------
    // Configuracion para el Puerto A
    //--------------------------------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);				//Habilitacion del Puerto A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);					//habilitacion de SSI0 para Touch resistivo (TSC2046)
    GPIOPinTypeSSI(GPIO_PORTA_BASE,GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
    //Supuestamente ya todos los pines SSI0 estan configurados como
    //SSI al momento de un reset, por lo que no configuro mas nada
    /*
     * PA2 -> CLK
     * PA3 -> Fss (CS)
     * PA4 -> RX (MISO)
     * PA5 -> TX (MOSI)
     */

    //--------------------------------------------------------------------------------------
    // Configuracion para el Puerto B
    //--------------------------------------------------------------------------------------
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);			//Habilitacion del puerto B
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,CARRIER_DETECT | ADDRESS_MATCH | DATA_READY);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,TX_EN | TRX_CE | PWR_UP);

    GPIOPinWrite(GPIO_PORTB_BASE,TX_EN | TRX_CE | PWR_UP,OFF);		//el modulo esta en stdby
    //
    // entradas y salidas segun circuito
    //

    //--------------------------------------------------------------------------------------
    // Configuracion SSI
    //--------------------------------------------------------------------------------------
    SSIConfigSetExpClk(SSI0_BASE,SysCtlClockGet(),SSI_FRF_MOTO_MODE_0,SSI_MODE_MASTER,SSI0_FREC,SSI0_DATAW);
    //Configuracion del modulo SSI0 para utilizar nrf905. Modo master, Protocolo Motorola (0,0), Bitrate 1[MHz] 8bit frame
    SSIEnable(SSI0_BASE);	//habilitacion del modulo SSI0

    strcpy(Mensaje,"Hola mundo como estas?");

    while(1)  {

    	GPIOPinWrite(GPIO_PORTB_BASE,PWR_UP,ON);
    	GPIOPinWrite(GPIO_PORTB_BASE,TX_EN,ON);

    	i = 0;
    	while(Mensaje[i] != 0)  {
    		SSIDataPut(SSI0_BASE,Mensaje[i]);
    		i++;
    	}

    	GPIOPinWrite(GPIO_PORTB_BASE,TRX_CE,ON);
    	SysCtlDelay(200);
    	GPIOPinWrite(GPIO_PORTB_BASE,TRX_CE,OFF);

    	SSIDataGet(SSI0_BASE,&Aux);

    	SysCtlDeepSleep();

    }
}
