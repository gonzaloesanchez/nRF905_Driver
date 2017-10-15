/*
 * Inicializacion.c
 *
 *  Created on: 09/01/2014
 *      Author: Gonza
 */

#include <msp430.h>
#include "Inicializacion.h"
#include "nRF905.h"




void	ConfigWDT(void)  {
	WDTCTL = WDTPW + WDTHOLD;				// Stop watchdog timer
}

void	FaultRoutine(void)  {
	while(1);
}

void	ConfigClocks(void)  {
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF)
		FaultRoutine();		                				// Si los valores de calibracion
 					                        				// estan borrados salta a FaultRoutine()

	BCSCTL1 = CALBC1_16MHZ; 								// Set range
	DCOCTL = CALDCO_16MHZ;  								// Set DCO step + modulation
	SetBit(BCSCTL3, LFXT1S_2);     								// LFXT1 = VLO
 	RstBit(IFG1, OFIFG);                       					// Clear OSCFault flag
 	SetBit(BCSCTL2, SELM_0 + DIVM_0 + DIVS_2);					// MCLK = DCO/1, SMCLK = DCO/4
 																//MCLK = 16MHz; SMCLK = 4MHz
}

/****************************************************************************
 * 	Utilizamos el Timer0A para generar el PWM para el cargador de baterias
 * **************************************************************************/
/*
void ConfigTimer0_A3(void)  {
	TA0CTL = TACLR;					//Hago un Clear al timer por las dudas

	TA0CCR0 = F25KHZ;					// Establece el periodo del PWM, dado por CCR0 (original 160)

	SetBit(TA0CCTL0, CCIE);			// Habilito la interrupccion por CCR0 (Modo Compare) Fin ciclo PWM, se debe muestrear
	SetBit(TA0CCTL1,OUTMOD_7);		//Salida reset set sobre el pin que se configure para tal fin
	TA0CCR1 = F25KHZ / 2;			// Establece un valor inicial como para que el duty sea un 50%


	SetBit(TA0CTL, TASSEL_2 + ID_0 + MC_0);	//Fuente de clock SMCLK. Divisor de entrada: 1. Modo: HALT

	 * Aqui se setea el timer como para que tenga un periodo de 25[KHz]. Vease que el submain clock es 4[MHz]
	 * entonces, 4[MHz] / 1 = 4[MHz] Esto es frecuencia de entrada al Timer. Seteado en UP mode y con
	 * CCR0 = 160 la frecuencia nos da perfectamente 25[KHz]. No tiene mucha resolucion y eso puede afectar un
	 * poco, pero creo que no va a ser tanto
}
*/

/****************************************************************************
 * 	Utilizamos el Timer1A para generar el PWM para las ramas del inversor
 * **************************************************************************/
/*
void	ConfigTimer1_A3(void)
{
	TA1CTL = TACLR;					//Hago un Clear al timer por las dudas

	TA1CCR0 = F100HZ;					// Establece el periodo del PWM, dado por CCR0 (original 10000)
											//Este periodo tiene que ser media senoidal SI O SI 100[Hz]

	SetBit(TA1CCTL0, CCIE);			// Habilito la interrupccion por CCR0 (Modo Compare) Fin medio ciclo
	SetBit(TA1CCTL1, CCIE);			// Habilito la interrupccion por CCR1 (Modo Compare) Flanco Ascendente
	SetBit(TA1CCTL2, CCIE);			// Habilito la interrupccion por CCR2 (Modo Compare) Flanco Descendente
	TA1CCR1 = F100HZ + 1;					// Establece un valor inicial para que no salte una interrupcion falsa
	TA1CCR2 = F100HZ + 1;					//Con estos valores JAMAS va a pasar una interrupcion porque son mayores a CCR0

	SetBit(TA1CTL, TASSEL_2 + ID_2 + MC_0);	//Fuente de clock SMCLK. Divisor de entrada: 4. Modo: HALT

	* Aqui se setea el timer como para que tenga un periodo de 100Hz. Vease que el submain clock es 4[MHz]
	 * entonces, 4[MHz] / 4 = 1[MHz] Esto es frecuencia de entrada al Timer. Seteado en UP mode y con
	 * CCR0 = 10000 la frecuencia nos da perfectamente 100[Hz]
}
*/

void ConfigPorts(void)  {

	SetBit(CHIP_ENABLE_NRF_OUT, CHIP_ENABLE_NRF);		//Ponemos en alto CSN antes de que refleje el valor en el pin
	SetBit(CHIP_ENABLE_NRF_DIR, CHIP_ENABLE_NRF);		//CS de nRF905 -> Salida

	RstBit(PWR_UP_OUT,PWR_UP);			//inicializamos PWR_UP en 0
	SetBit(PWR_UP_DIR,PWR_UP);			//PWR_UP -> Salida

	RstBit(TRX_CE_OUT,TRX_CE);			//inicializamos TRX_CE en 0
	SetBit(TRX_CE_DIR,TRX_CE);			//TRX_CE -> Salida

	RstBit(TX_EN_OUT,TX_EN);			//inicializamos TX_EN en 0
	SetBit(TX_EN_DIR,TX_EN);			//TX_EN -> Salida

	RstBit(DATA_READY_DIR,DATA_READY);				//DATA_READY -> Entrada
	RstBit(ADDRESS_MATCH_DIR,ADDRESS_MATCH);		//ADDRESS_MATCH -> Entrada
	RstBit(CARRIER_DETECT_DIR,CARRIER_DETECT);	//CARRIER_DETECT -> Entrada

	SetBit(P1SEL, BIT5|BIT6|BIT7);					//Configuraciones para pines de SPI
	SetBit(P1SEL2, BIT5|BIT6|BIT7);					//Configuraciones para pines de SPI

}

/*
void ConfigUART(void)  {
	P1SEL |= (BIT1 + BIT2) ;                  // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= (BIT1 + BIT2);

	UCA0CTL1 = UCSSEL_2 | UCSWRST;            // SMCLK	--> 4[Mhz]
	UCA0CTL0 = UCMODE_0;                      // UART-Mode Async LSB 8Bit 1Stop NoParity
	UCA0BR0 = ( 416    )&0xff;               // 4[MHz] 9600
	UCA0BR1 = ( 416>>8 )&0xff;               // 4[MHz] 9600
	UCA0MCTL = UCBRF_0 | UCBRS_6;             // UCOS16=0
	UCA0IRTCTL = 0x00;                        // IrDA desactivated
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IFG2 &= ~UCA0RXIE;
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}
*/


void ConfigSPI(void)  {
	UCB0CTL1 |= UCSSEL_2 + UCSWRST;				//Source clock: SMCLK. Software reset held
	RstBit(UCB0CTL0,UCCKPH+UCCKPL+UC7BIT+UCMODE_0);	//SPI modo 0;0, 8bits, SPI 3 pines
	SetBit(UCB0CTL0,UCMSB+UCMST+UCSYNC);		//MsB sale primero, Master Mode, SYNC=1 necesario
	UCB0BR0 = 4;										// Valor para que el bitrate = 1MHz
	UCB0BR1 = 0;										// Valor para que el bitrate = 1MHz

	RstBit(UCB0CTL1,UCSWRST);			//SPI listo para ser activado
	SetBit(IE2,UCB0RXIE);				//interrupcion por RX completo. Esta interrupcion ocurre cada vez que
												//un dato se recibe completo. No utilizo la TX porque esta salta
												//cuando se vacia el TXBUFF, y esta dobleBuffered (O sea, salta al principio
												//de la transaccion, no al final). Ambas operaciones (Tx y Rx) son concurrentes
}


/*
void	ConfigADC10(void)  {

	SetBit(ADC10CTL1, INCH_4 + ADC10DIV_0 + ADC10SSEL_3 + CONSEQ_0);// Se selecciona el canal A4 para hacer conversion de bloque
																			// Se configura como señal de trigger (SAMPCON) el bit ADC10ST
																			// Seleccion del clock: SMCLK. Division del Clock: /1;
																			// Modo conversion simple, en este caso "sigle channel"
	SetBit(ADC10AE0,SENSADO_V + SENSADO_I);						//Enable analog inputs

	SetBit(ADC10CTL0, SREF_1 + ADC10SHT_3 + ADC10ON + ADC10IE);				// Fuente de la referencia: VR+ = VREF+ and VR- = VSS
	SetBit(ADC10CTL0,REF2_5V + REFON);										// Tiempo de Sample and Hold: 64xClock
																			// Se enciende la referencia interna (2.5V setting REF2_5V)
																			// Se enciende el ADC y se habilita la interrupcion al final de la secuencia
																			// de conversion.
	_delay_cycles(640);			// 40 us con MCLK 16 MHz para establecer Vref

	ConvActual = eTension;		//Seteos iniciales
	CambiarCanal(eTension);

	SetBit(ADC10CTL0, ENC);			// ADC Habilitado
}
*/
