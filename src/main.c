#include <msp430.h>
#include "nRF905.h"
#include "Inicializacion.h"

/*
 * main.c
 */

//INTERRUPCIÓN UART RX
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)  {

	uint8_t Aux;

	Aux = IFG2;
	if ((Aux & UCB0RXIFG) != 0)  {
		Aux = UCB0RXBUF;				//solamente para limpiar la bandera de interrupcion
		setSPI_IRQFlag();				//Ocurrio la interrupcion de finalizacion de operacion
											//seteamos la bandera correspondiente
		 __low_power_mode_off_on_exit();
	}
}


int main(void) {
	uint8_t aux[32];
	uint8_t i;

	aux[0] = 8;
	for(i=1;i<8;i++)  {
		aux[i] = i*2;
	}


	ConfigWDT();			// Detengo el WachDog
	ConfigClocks();		// Configuro el Clock a 16MHz
	ConfigPorts();			//Configura los puertos
	ConfigSPI();
	_enable_interrupts();		//Habilitamos las interrupciones globales

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
