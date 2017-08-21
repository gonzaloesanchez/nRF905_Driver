/*
 * nRF905.c
 *
 *  Created on: 21/08/2017
 *      Author: Gonza
 */

#include "nRF905.h"

/**
 * Definiciones globales de este modulo, no visibles hacia afuera
 */

static uint8_t g_ui8StatusReg;	//esta variable contiene status despues de cada
								//transicion bajo-alto de CD

nRF905 g_nRF905_Config;			//variable global que contiene toda la configuracion
								//del dispositivo.
								//Todas las funciones de configuracion tendran esta
								//estructura como argumento

/**--------------------------------------------------------------------------
 * Funciones de bajo nivel. Estas son las que deben ser re-definidas para
 * cada plataforma de HW en las que se porte el codigo
 * --------------------------------------------------------------------------
 */

/**
 * Esta funcion es la que se encarga de enviar los datos por el HW SPI
 * Toma como argumento el valor a enviar
 * Devuelve true si el envio fue exitoso o false en caso contrario
 * Esta funcion se implementa con funciones NO BLOQUEANTES
 */
static bool spi_send(uint8_t S)  {
	bool Ret = false;

	//Esqueleto de la funcion para enviar por SPI
	//TODO: Definir para cada HW
	//Las causas de falla normalmente son que no haya lugar en el FIFO
#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}

/**
 * Esta funcion es la que se encarga de recibir los datos por el HW SPI
 * Toma como argumento un puntero a la variable que contendra el valor recibido
 * Devuelve true si la recepcion fue exitosa o false en caso contrario
 * Esta funcion se implementa con funciones NO BLOQUEANTES
 */
static bool spi_receive(uint8_t *R)  {
	bool Ret = false;

	//Esqueleto de la funcion para recibir por SPI
	//TODO: Definir para cada HW
	//Las causas de falla normalmente son que no haya nada en el FIFO
#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}

/**
 * Esta funcion es la que se encarga de setear el pin correspondiente a TX_EN
 */
static void setTX_Enable(bool Value)  {
#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif
}

/**
 * Esta funcion es la que se encarga de setear el pin correspondiente a TRX_CE
 */
static void setTRX_ChipEnable(bool Value)  {
#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif
}

/**
 * Esta funcion es la que se encarga de setear el pin correspondiente a PWR_UP
 */
static void setPowerUp(bool Value)  {

#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

}



static bool getAddressMatch(void)  {
	bool Ret;

#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}

static bool getDataReady(void)  {
	bool Ret;

#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}

static bool getCarrierDetect(void)  {
	bool Ret;

#ifdef TIVA_C
#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}

static bool getAddressMatch_FromIRQ(void)  {
	bool Ret = false;

	if(g_nRF905_Config.AM)
		Ret = true;

	//TODO: Si es necesario, codificar aqui para HW limitado

	return Ret;
}

static bool getDataReady_FromIRQ(void)  {
	bool Ret = false;

	if(g_nRF905_Config.DR)
		Ret = true;

	//TODO: Si es necesario, codificar aqui para HW limitado

	return Ret;
}

static bool getCarrierDetect_FromIRQ(void)  {
	bool Ret = false;

	if(g_nRF905_Config.CD)
		Ret = true;

	//TODO: Si es necesario, codificar aqui para HW limitado

	return Ret;
}



/*
 * /**
 * Esta funcion devuelve el valor de la variable interna al modulo. Asi la hacemos
 * solo de escritura para las funciones exteriores

bool getStatusReg(uint8_t *p_ui8status)  {
	bool Ret = false;
	uint8_t Aux;

	//se hace una lectura SPI. Si hay algo en el FIFO esta funcion devuelve true
	//asi que cargamos el valor a la variable pasada por referencia
	if (spi_receive(&Aux))  {
		*p_ui8status = Aux;
		Ret = true;
	}
	//caso contrario esta funcion no altera el valor de la variable argumento
	//Es importante, al ser por referencia
	return Ret;
}
 */
 */
