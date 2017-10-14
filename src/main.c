/* Copyright 2014, 2015 Mariano Cerdeiro
 * Copyright 2014, Pablo Ridolfi
 * Copyright 2014, Juan Cecconi
 * Copyright 2014, Gustavo Muro
 * Copyright 2015, Eric Pernia
 * All rights reserved.
 *
 * This file is part of CIAA Firmware.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** \brief Bare Metal example source file
 **
 ** This is a mini example of the CIAA Firmware.
 **
 **/

/** \addtogroup CIAA_Firmware CIAA Firmware
 ** @{ */
/** \addtogroup Examples CIAA Firmware Examples
 ** @{ */
/** \addtogroup Baremetal Bare Metal example source file
 ** @{ */

/*
 * Initials    Name
 * ---------------------------
 * ENP          Eric Pernia
 *
 */

/*
 * modification history (new versions first)
 * -----------------------------------------------------------
 * 20151104   v0.0.1   ENP   First version
 */

/*==================[inclusions]=============================================*/
#include "main.h"
#include "nRF905.h"

#ifndef CPU
#error CPU shall be defined
#endif
#if (lpc4337 == CPU)
#include "chip.h"
#elif (mk60fx512vlq15 == CPU)
#else
#endif

/*==================[macros and definitions]=================================*/


#define INPUT		0
#define OUTPUT		1

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

volatile uint32_t msTicks = 0;


/*==================[internal functions definition]==========================*/


static void initHardware(void)  {
	Chip_GPIO_Init(LPC_GPIO_PORT);

	//Inicializamos el led RGB
   /* Config EDU-CIAA-NXP Led Pins as GPIOs */
   Chip_SCU_PinMux(LEDR_P, LEDR_P_, MD_PUP, FUNC4); /* P2_0,  GPIO5[0],  LEDR */
   Chip_SCU_PinMux(LEDG_P, LEDG_P_, MD_PUP, FUNC4); /* P2_1,  GPIO5[1],  LEDG */
   Chip_SCU_PinMux(LEDB_P, LEDB_P_, MD_PUP, FUNC4); /* P2_2,  GPIO5[2],  LEDB */
   Chip_SCU_PinMux(LED1_P, LED1_P_, MD_PUP, FUNC0); /* P2_10, GPIO0[14], LED1 */
   Chip_SCU_PinMux(LED2_P, LED2_P_, MD_PUP, FUNC0); /* P2_11, GPIO1[11], LED2 */
   Chip_SCU_PinMux(LED3_P, LED3_P_, MD_PUP, FUNC0); /* P2_12, GPIO1[12], LED3 */

   /* Config EDU-CIAA-NXP Led Pins as Outputs */
   Chip_GPIO_SetDir(LPC_GPIO_PORT, LEDR_GPIO, (1<<LEDR_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, LEDG_GPIO, (1<<LEDG_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, LEDB_GPIO, (1<<LEDB_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, LED1_GPIO, (1<<LED1_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, LED2_GPIO, (1<<LED2_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, LED3_GPIO, (1<<LED3_PIN), OUTPUT);

   /* Init EDU-CIAA-NXP Led Pins OFF */
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, LEDR_GPIO, (1<<LEDR_PIN));
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, LEDG_GPIO, (1<<LEDG_PIN));
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, LEDB_GPIO, (1<<LEDB_PIN));
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, LED1_GPIO, (1<<LED1_PIN));
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, LED2_GPIO, (1<<LED2_PIN));
   Chip_GPIO_ClearValue(LPC_GPIO_PORT, LED3_GPIO, (1<<LED3_PIN));

   /* Config EDU-CIAA-NXP Button Pins as GPIOs */
   Chip_SCU_PinMux(TEC1_P, TEC1_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_0,  GPIO0[4], TEC1 */
   Chip_SCU_PinMux(TEC2_P, TEC2_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_1,  GPIO0[8], TEC2 */
   Chip_SCU_PinMux(TEC3_P, TEC3_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_2,  GPIO0[9], TEC3 */
   Chip_SCU_PinMux(TEC4_P, TEC4_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P1_6,  GPIO1[9], TEC4 */

   /* Config EDU-CIAA-NXP Button Pins as Inputs */
   Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC1_GPIO, (1<<TEC1_PIN), INPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC2_GPIO, (1<<TEC3_PIN), INPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC2_GPIO, (1<<TEC2_PIN), INPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, TEC4_GPIO, (1<<TEC4_PIN), INPUT);

   /* Read clock settings and update SystemCoreClock variable */
   SystemCoreClockUpdate();

   /* To configure the Systick timer we use the SysTick_Config(uint32_t ticks);
    * funtion. With ticks we can set the Systick timer interval. In our case we
    * have a 204 MHz clock and we want it to fire each ms. So the tick has to
    * be set to 204 MHz / 1000.
    */
   SysTick_Config( SystemCoreClock / TICKRATE_HZ); /* TICKRATE_HZ ticks per second */

   /**
    * Configuraciones de pines especificas de este proyecto
    */
   Chip_SCU_PinMux(PWR_UP_P, PWR_UP_P_, MD_PUP, FUNC0); 							/* P6_1,  GPIO3[0],  PWR_UP */
   Chip_SCU_PinMux(ADDRESS_MATCH_P, ADDRESS_MATCH_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); /* P6_4,  GPIO3[3],  ADDRESS_MATCH */
   Chip_SCU_PinMux(DATA_READY_P, DATA_READY_P_, MD_PUP|MD_EZI|MD_ZI, FUNC0); 		/* P6_5,  GPIO3[4],  DATA_READY */
   Chip_SCU_PinMux(CARRIER_DETECT_P, CARRIER_DETECT_P_, MD_PUP|MD_EZI|MD_ZI, FUNC4); 	/* P6_7,  GPIO5[15], CARRIER_DETECT */
   Chip_SCU_PinMux(CHIP_ENABLE_P, CHIP_ENABLE_P_, MD_PUP, FUNC4);					/* P6_8,  GPIO5[16], CHIP_SELECT */
   Chip_SCU_PinMux(TRX_CE_P, TRX_CE_P_, MD_PUP, FUNC0);						/* P6_10,  GPIO3[6], TRX_CE */
   Chip_SCU_PinMux(TX_EN_P, TX_EN_P_, MD_PUP, FUNC0);							/* P6_9,  GPIO3[5], TX_EN */

   //Salidas => PWR_UP, CHIP_SELECT, TRX_CE, TX_EN
   Chip_GPIO_SetDir(LPC_GPIO_PORT, PWR_UP_GPIO, (1<<PWR_UP_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, CHIP_ENABLE_GPIO, (1<<CHIP_ENABLE_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, TX_EN_GPIO, (1<<TX_EN_PIN), OUTPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, TRX_CE_GPIO, (1<<TRX_CE_PIN), OUTPUT);

   //Entradas => ADDRESS_MATCH, DATA_READY, CARRIER_DETECT
   Chip_GPIO_SetDir(LPC_GPIO_PORT, ADDRESS_MATCH_GPIO, (1<<ADDRESS_MATCH_PIN), INPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, DATA_READY_GPIO, (1<<DATA_READY_PIN), INPUT);
   Chip_GPIO_SetDir(LPC_GPIO_PORT, CARRIER_DETECT_GPIO, (1<<CARRIER_DETECT_PIN), INPUT);


   /**
    * Configuracion para el puerto SPI
    * Set up clock and power for SSP1 module
    * Configure SSP SSP1 pins
    */
   Chip_SCU_PinMuxSet(0xf, 4, (SCU_MODE_PULLUP | SCU_MODE_FUNC0)); // CLK0
   Chip_SCU_PinMuxSet(0x1, 3, (SCU_MODE_PULLUP | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC5)); // MISO1
   Chip_SCU_PinMuxSet(0x1, 4, (SCU_MODE_PULLUP | SCU_MODE_FUNC5)); // MOSI1

   Chip_SCU_PinMuxSet(0x6, 1, (SCU_MODE_PULLUP | SCU_MODE_FUNC0)); // CS1 configured as GPIO
   Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 3, 0);

   // Initialize SSP Peripheral
   Chip_Clock_Enable(CLK_MX_SSP1);			//No me preguntes, esto asi anda
   Chip_Clock_Enable(CLK_APB2_SSP1);
   Chip_SSP_Set_Mode(LPC_SSP1, SSP_MODE_MASTER);
   Chip_SSP_SetFormat(LPC_SSP1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_CPHA0_CPOL0);
   Chip_SSP_SetBitRate(LPC_SSP1, 1000);
   Chip_SSP_Enable( LPC_SSP1 );
}

/* blocks for dlyTicks ms */
__INLINE static void delay(uint32_t dlyTicks){
	uint32_t curTicks;

	curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

/*==================[external functions definition]==========================*/

__attribute__ ((section(".after_vectors")))
void SysTick_Handler(void) {

	msTicks++;
}



/** \brief Main function
 *
 * This is the main entry point of the software.
 *
 * \returns 0
 *
 * \remarks This function never returns. Return value is only to avoid compiler
 *          warnings or errors.
 */
int main(void)  {
	uint8_t aux[32];
	uint8_t i;

	initHardware();

	aux[0] = 8;
	for(i=1;i<8;i++)  {
		aux[i] = i*3;
	}

	nRF905_Init();
	nRF905_setTXAddress(0x123B56FF);
	nRF905_ChanelConfig();
	nRF905_RxPayload_rd(aux,8);
	nRF905_TxPayload_wr(aux,8);

	nRF905_WriteConfig();

	while(1) {

	}

	return 0;
}

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
/*==================[end of file]============================================*/
