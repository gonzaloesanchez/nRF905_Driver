#include <msp430.h>
#include "inc/nRF905.h"
#include "inc/Inicializacion.h"

/*
 * main.c
 */

//INTERRUPCIÓN UART RX
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)  {

	uint8_t Aux;

	Aux = IFG2;
	if ((Aux & UCB0RXIFG) != 0)  {
		setSPI_IRQFlag();				//Ocurrio la interrupcion de finalizacion de operacion
	}										//seteamos la bandera correspondiente
}


int main(void) {
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
