/*
 * nRF905.c
 *
 *  Created on: 21/08/2017
 *      Author: Gonza
 */

/**
 * Este modulo define las funciones necesarias para implementar comunicaciones
 * con el modulo RF basado en el chip nRF905 de NORDIC. Se deben tener ciertos
 * cuidados para cada plataforma: Esta libreria utiliza interrupciones, asi que
 * estas deben ser configuradas fuera de la libreria, para cada plataforma. Eso
 * da mas claridad y generalidad a la misma (y el control del hardware lo tiene
 * el programador).
 * Al utilizar interrupciones y modos de bajo consumo, es necesario que se
 * configure adecuadamente el clocking al HW del SPI (algunos modos de bajo consumo
 * quitan la fuente de clock a los perifericos para ahorrar energia)
 *
 */


/*
 * TODO: Dibujar flujo de datos entre modulo y funciones externas. Esto va a ser
 * aclaratorio sobre cuales funciones deben ser utilizadas en las interrupciones
 * o en el software que las simule
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

static spi_flags g_spi_Control;		//esta es una variable global que debe ser modificada
									//por las interrupciones para las funciones base
									//de esta libreria. La modificacion solo se hara
									//mediante funciones set desde las interrupciones

/**--------------------------------------------------------------------------
 * Funciones de bajo nivel. Estas son las que deben ser re-definidas para
 * cada plataforma de HW en las que se porte el codigo
 *
 * ATENCION: Cabe aclarar que no se puede usar por hardware el CS del SPI,
 * porque la trama que implementa Nordic no es estandar (esto es, no cierra con
 * pulsos del CS cada paquete de 8 bits). Esto obliga a implementarlo por
 * software y utilizar interrupciones para detectar tanto la recepcion como
 * la transmision exitosas de un paquete, para no quedar en un while esperando
 * que se concreten. Esto esta mas que nada pensado para los nodos que estan
 * alimentados a bateria.
 * --------------------------------------------------------------------------
 */

/**
 * Esta funcion es la que se encarga de enviar los comandos por el HW SPI, y
 * debe ser implementada para cada HW distinto.
 * Toma como argumento el valor a enviar.
 * Devuelve true si el envio fue exitoso, o false en caso contrario.
 * Esta funcion se implementa con funciones NO BLOQUEANTES.
 * Las causas de falla normalmente son que no haya lugar en el FIFO.
 *
 * 		USAR INTERRUPCIONES
 * 		while(variable_global)  {
 * 			WFI;
 * 		}
 * 		Variable global podra ser cambiada por software para dar portabilidad (esto es
 * 		solo para cuando se documente, para el final)
 */
static bool spi_write(uint8_t Comando,uint16_t Dato)  {
	bool Ret = false;
	uint32_t Aux;

#ifdef TIVA_C

	GPIOPinWrite(CHIP_ENABLE_BASE,CHIP_ENABLE_GPIO,OFF);	//CS -> Low (Habilitacion Negada)
	if(!SSIDataPutNonBlocking(SPI_BASE,(uint32_t)S))		//Ponemos el Comando en el buffer FIFO
		return Ret;										//Si esta funcion devuelve != 0 el
														//envio se iniciara en breves instantes
														//caso contrario debemos salir, es una excepcion

	while (!g_spi_Control.irqTX)  {			//esperamos un evento
		SysCtlDeepSleep();					//esta funcion tiene un WFI dentro
	}
	g_spi_Control.irqTX = false;			//limpiamos el flag

	//Ponemos la parte baja del Dato en el buffer FIFO. Si falla debemos salir, excepcion de
	//envio
	if(!SSIDataPutNonBlocking(SPI_BASE,(uint32_t) (0x00FF & Dato)))
		return Ret;

	while (!g_spi_Control.irqTX)  {			//esperamos un evento
		SysCtlDeepSleep();					//esta funcion tiene un WFI dentro
	}
	g_spi_Control.irqTX = false;			//limpiamos el flag

	//Ponemos la parte alta del Dato en el buffer FIFO. Si falla debemos salir, excepcion de
	//envio
	if(!SSIDataPutNonBlocking(SPI_BASE,(uint32_t) (Dato >> 8)))
		return Ret;

	//Debemos sacar el byte correspondiente a STATUS_REG. Si falla es una excepcion
	if (!SSIDataGetNonBlocking(SPI_BASE,&Aux))
		return Ret;
	g_ui8StatusReg = (uint8_t)Aux;			//actualizacion de status reg

	while(SSIDataGetNonBlocking(SPI_BASE,&Aux));	//sacamos toda la basura del FIFO

	Ret = true;				//si llegamos hasta aqui, la transaccion fue exitosa
	return Ret;


#endif


#ifdef MSP430
#endif


#ifdef EDU_CIAA
#endif

	return Ret;
}

/**
 * Esta funcion es la que se encarga de recibir los datos por el HW SPI,
 * haciendo un read.
 * Toma como argumento un puntero a la variable que contendra el valor recibido.
 * Devuelve true si la recepcion fue exitosa, o false en caso contrario.
 * Esta funcion se implementa con funciones NO BLOQUEANTES.
 * Las causas de falla normalmente son que no haya nada en el FIFO
 */

/* TODO: Hay que cambiar las funcion spi_receive por spi_read, para
 * 		abarcar una transaccion completa. Ver Notas
 * 		USAR INTERRUPCIONES
 * 		while(variable_global)  {
 * 			WFI;
 * 		}
 * 		Variable global podra ser cambiada por software para dar portabilidad (esto es
 * 		solo para cuando se documente, para el final)
 */
static bool spi_receive(uint8_t *R)  {
	bool Ret = false;
	uint32_t Aux;

#ifdef TIVA_C

	if(SSIDataGetNonBlocking(SPI_BASE,&Aux))  {			//Se piden datos desde el FIFO
		*R = (uint8_t)Aux;							//si los datos estan se pasan a la variable
		Ret = true;							//por referencia y se devuelve TRUE
	}


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

	if(Value)
		GPIOPinWrite(TX_EN_BASE,TX_EN_GPIO,ON);
	else
		GPIOPinWrite(TX_EN_BASE,TX_EN_GPIO,OFF);

#endif


#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif
}

/*****************************************************************************************
 * Funcion getAddressMatch
 * Esta funcion es la que se encarga de setear el pin correspondiente a TRX_CE
 *****************************************************************************************/
static void setTRX_ChipEnable(bool Value)  {

#ifdef TIVA_C

	if(Value)
		GPIOPinWrite(TRX_CE_BASE,TRX_CE_GPIO,ON);
	else
		GPIOPinWrite(TRX_CE_BASE,TRX_CE_GPIO,OFF);

#endif

#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif
}


/*****************************************************************************************
 * Funcion getAddressMatch
 * Esta funcion es la que se encarga de setear el pin correspondiente a PWR_UP
 *****************************************************************************************/
static void setPowerUp(bool Value)  {

#ifdef TIVA_C

	if(Value)
		GPIOPinWrite(PWR_UP_BASE,PWR_UP_GPIO,ON);
	else
		GPIOPinWrite(PWR_UP_BASE,PWR_UP_GPIO,OFF);

#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

}


/*****************************************************************************************
 * Funcion getAddressMatch
 *****************************************************************************************/
static bool getAddressMatch(void)  {
	bool Ret;
	uint32_t Pins;

#ifdef TIVA_C

	Pins = GPIOPinRead(ADDRESS_MATCH_BASE,ADDRESS_MATCH_GPIO);		//leemos el valor del puerto
	if(ADDRESS_MATCH_GPIO & Pins)					//ese valor viene enmascarado por ADDRESS_MATCH_GPIO
		Ret = true;									//por lo que si es HIGH el resultado es != 0
	else
		Ret = false;								//caso contrario sera 0

#endif

#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}


/*****************************************************************************************
 * Funcion getDataReady
 *****************************************************************************************/
static bool getDataReady(void)  {
	bool Ret;
	uint32_t Pins;

#ifdef TIVA_C

	Pins = GPIOPinRead(DATA_READY_BASE,DATA_READY_GPIO);		//leemos el valor del puerto
	if(DATA_READY_GPIO & Pins)					//ese valor viene enmascarado por DATA_READY_GPIO
		Ret = true;									//por lo que si es HIGH el resultado es != 0
	else
		Ret = false;								//caso contrario sera 0

#endif

#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}


/*****************************************************************************************
 * Funcion getCarrierDetect
 *****************************************************************************************/
static bool getCarrierDetect(void)  {
	bool Ret;
	uint32_t Pins;

#ifdef TIVA_C

	Pins = GPIOPinRead(CARRIER_DETECT_BASE,CARRIER_DETECT_GPIO);		//leemos el valor del puerto
	if(CARRIER_DETECT_GPIO & Pins)					//ese valor viene enmascarado por CARRIER_DETECT_GPIO
		Ret = true;									//por lo que si es HIGH el resultado es != 0
	else
		Ret = false;								//caso contrario sera 0

#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

	return Ret;
}


/*****************************************************************************************
 * Funcion getAddressMatch_FromIRQ
 *****************************************************************************************/
static bool getAddressMatch_FromIRQ(void)  {
	bool Ret = false;

	if(g_nRF905_Config.AM)
		Ret = true;

	//TODO: Si es necesario, codificar aqui para HW limitado

	return Ret;
}


/*****************************************************************************************
 * Funcion getDataReady_FromIRQ
 *****************************************************************************************/
static bool getDataReady_FromIRQ(void)  {
	bool Ret = false;

	if(g_nRF905_Config.DR)
		Ret = true;

	//TODO: Si es necesario, codificar aqui para HW limitado

	return Ret;
}


/*****************************************************************************************
 * Funcion getCarrierDetect_FromIRQ
 *****************************************************************************************/
static bool getCarrierDetect_FromIRQ(void)  {
	bool Ret = false;

	if(g_nRF905_Config.CD)
		Ret = true;

	//TODO: Si es necesario, codificar aqui para HW limitado

	return Ret;
}

/*----------------------------------------------------------------------------------------

		LA PROXIMA SECCION CONTIENE LAS FUNCIONES ACCESIBLES POR MODULOS EXTERNOS

----------------------------------------------------------------------------------------*/

/*
 * Esta funcion devuelve el valor de la variable interna al modulo. Asi la hacemos
 * solo de escritura para las funciones exteriores.
 */
bool nRF905_getStatusReg(uint8_t *status)  {
	bool Ret = false;
	uint8_t Aux;

	//se hace una lectura SPI. Si hay algo en el FIFO esta funcion devuelve true
	//asi que cargamos el valor a la variable pasada por referencia
	if (spi_receive(&Aux))  {
		*status = Aux;
		Ret = true;
	}
	//caso contrario esta funcion no altera el valor de la variable argumento
	//Es importante, al ser por referencia
	return Ret;
}

/*
 * Esta funcion inicia el estado de CS del SPI y las variables de la estructura de datos
 * que controla el estado del modulo con valores por defecto
 */
void nRF905_Init(void)  {

	//TODO: Modificar valores!! Definir constantes para cada caso.
	//Estos valores no son correctos
	g_nRF905_Config.Canal = 0;
	g_nRF905_Config.ClockModulo = 0;
	g_nRF905_Config.ClockOut = false;
	g_nRF905_Config.DireccionRX = 0xFFFFFFFF;
	g_nRF905_Config.DireccionTX = 0xFFFFFFFF;
	g_nRF905_Config.LongRX_Payload = 0;
	g_nRF905_Config.LongTX_Payload = 0;
	g_nRF905_Config.PLL_Freq = 0;
	g_nRF905_Config.Potencia = 0;
	g_nRF905_Config.Retransmision = false;


#ifdef TIVA_C

	GPIOPinWrite(CHIP_ENABLE_BASE,CHIP_ENABLE_GPIO,ON);	//CS -> High (Deshabilitamos)

#endif
#ifdef MSP430
#endif
#ifdef EDU_CIAA
#endif

}

/**
 * Funcion para setear la bandera de interrupcion de RX. La encapsulo para no tener
 * variables globales definidas en este modulo dando vueltas en el proyecto
 */
void nRF905_setRXFlag(void)  {
	g_spi_Control.irqRX = true;
}

/**
 * Funcion para setear la bandera de interrupcion de TX. La encapsulo para no tener
 * variables globales definidas en este modulo dando vueltas en el proyecto
 */
void nRF905_setTXFlag(void)  {
	g_spi_Control.irqTX = true;
}
