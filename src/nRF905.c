/*
 * nRF905.c
 *
 *  Created on: 21/08/2017
 *      Author: Gonza
 */

/**
 * @brief Modulo que implementa el driver para nRF905.
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
 * @brief Definiciones globales de este modulo, no visibles hacia afuera
 */

static uint8_t g_ui8StatusReg;	//esta variable contiene status despues de cada
								//transicion bajo-alto de CD


static nRF905 g_nRF905_Config;	//variable global que contiene toda la configuracion
								//del dispositivo.
								//Todas las funciones de configuracion tendran esta
								//estructura como argumento

static bool g_spi_IRQFlag;		//esta es una variable global que debe ser seteada
								//por las interrupciones para las funciones base
								//de esta libreria. La modificacion solo se hara
								//mediante funciones set desde las interrupciones



/*--------------------------------------------------------------------------
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
 *
 * @code
 * 		//USAR INTERRUPCIONES
 * 		while(variable_global)  {
 * 			WFI;
 * 		}
 * @endcode
 * @p variable_global podra ser cambiada por software para dar portabilidad
 * --------------------------------------------------------------------------
 */


/**
 * @brief Esta funcion es la que se encarga de enviar los comandos por el HW SPI.
 *
 * La funcion debe ser implementada para cada HW distinto.
 * @param Comando Es el comando a enviar. Los comandos son: (Agregar tabla de comandos)
 * @param Dato Es el parametro a enviar para configurar el modulo nRF905
 * @return La funcion devuelve @p TRUE si el envio fue exitoso o @p FALSE en caso contrario
 * @note Esta funcion se implementa con funciones NO BLOQUEANTES.
 * @warning Las causas de falla normalmente son que no haya lugar en el FIFO.
 */
static bool spi_write(uint8_t Comando)  {
	bool Ret = false;
	uint8_t Aux;


#ifdef MSP430

	UCB0TXBUF = Comando;					//Comienza la transmision
	while (!g_spi_IRQFlag)  {			//esperamos un evento
			LPM3;								//entramos en LowPowerMode. Es una macro esto
	}
	g_spi_IRQFlag = false;

	Ret = true;

#endif


#ifdef EDU_CIAA

	Chip_SSP_DATA_SETUP_T xferConfig;

	xferConfig.tx_data = &Comando;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = NULL;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = 1;

	Chip_SSP_RWFrames_Blocking( LPC_SSP1, &xferConfig );

	Ret = true;

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

/* 		Hay que cambiar las funcion spi_receive por spi_read, para
 * 		abarcar una transaccion completa. Ver Notas
 * 		USAR INTERRUPCIONES
 * 		while(variable_global)  {
 * 			WFI;
 * 		}
 * 		Variable global podra ser cambiada por software para dar portabilidad (esto es
 * 		solo para cuando se documente, para el final)
 */
static bool spi_read(uint8_t *R)  {
	bool Ret = false;
	uint32_t Aux;


#ifdef MSP430
	/*
	 * No me sirve de mucho en esta arquitectura porque solamente tiene un byte de buffer
	 * de todas maneras no lo uso para nada al read (no se implemento ninguna funcion de
	 * lectura)
	 * */
	*R = UCB0RXBUF;	//devolvemos el dato leido
	Ret = true;

#endif
#ifdef EDU_CIAA

	Chip_SSP_DATA_SETUP_T xferConfig;

	xferConfig.tx_data = NULL;
	xferConfig.tx_cnt  = 0;
	xferConfig.rx_data = R;
	xferConfig.rx_cnt  = 0;
	xferConfig.length  = 1;

	Chip_SSP_RWFrames_Blocking( LPC_SSP1, &xferConfig );

	Ret = true;

#endif

	return Ret;
}

/*****************************************************************************************
 * Funcion spi_flush
 * Sirve para limpiar el FIFO de entrada del HW SPI
 *****************************************************************************************/
static void spi_flush(void)  {
	uint8_t Aux;

#ifdef MSP430
	Aux = UCB0RXBUF;	//esto provoca que la bandera RXIF se limpie por HW
#endif
#ifdef EDU_CIAA

	/* Clear all remaining frames in RX FIFO */
	while (Chip_SSP_GetStatus(LPC_SSP1, SSP_STAT_RNE)) {
		Chip_SSP_ReceiveFrame(LPC_SSP1);
	}

#endif
}

/*****************************************************************************************
 * Funcion spi_wait
 * Sirve para esperar eventos de completamiento del SPI (Hasta ahora solo para TIVA)
 * (no es necesario en MSP430)
 *****************************************************************************************/
static void spi_wait(void)  {

#ifdef MSP430
	//no hace falta, la interrupcion maneja esto
#endif
#ifdef EDU_CIAA
	uint32_t i;

	/*
	 * Toda la transmision de SPI es bloqueante con las funciones utilizadas, asi que esto
	 * no afecta. Es para que el CS no se ponga en idle antes de cumplido el Tcch despues
	 * del ultimo clock. La constante es empirica
	 */
	for(i=0;i<TCCH_NRF905;i++);

#endif
}

/**
 * Esta funcion es la que se encarga de setear el pin correspondiente a TX_EN
 */
void setTX_Enable(bool Value)  {

#ifdef MSP430

	if (Value)
		SetBit(TX_EN_OUT,TX_EN);
	else
		RstBit(TX_EN_OUT,TX_EN);

#endif
#ifdef EDU_CIAA

	Chip_GPIO_SetPinState( LPC_GPIO_PORT, TX_EN_GPIO, TX_EN_PIN, Value);

#endif
}

/*****************************************************************************************
 * Funcion getAddressMatch
 * Esta funcion es la que se encarga de setear el pin correspondiente a TRX_CE
 *****************************************************************************************/
void setTRX_ChipEnable(bool Value)  {


#ifdef MSP430

	if (Value)
		SetBit(TRX_CE_OUT,TRX_CE);
	else
		RstBit(TRX_CE_OUT,TRX_CE);

#endif
#ifdef EDU_CIAA

	Chip_GPIO_SetPinState( LPC_GPIO_PORT, TRX_CE_GPIO, TRX_CE_PIN, Value);

#endif
}


/*****************************************************************************************
 * Funcion getAddressMatch
 * Esta funcion es la que se encarga de setear el pin correspondiente a PWR_UP
 *****************************************************************************************/
void setPowerUp(bool Value)  {

#ifdef MSP430

	if (Value)
		SetBit(PWR_UP_OUT,PWR_UP);
	else
		RstBit(PWR_UP_OUT,PWR_UP);

#endif
#ifdef EDU_CIAA

	Chip_GPIO_SetPinState( LPC_GPIO_PORT, PWR_UP_GPIO, PWR_UP_PIN, Value);

#endif

}

/*****************************************************************************************
 * Funcion getAddressMatch
 * Esta funcion es la que se encarga de setear el pin correspondiente a PWR_UP
 *****************************************************************************************/
void setChipEnable(bool Value)  {

#ifdef MSP430

	if (Value)
		SetBit(CHIP_ENABLE_NRF_OUT,CHIP_ENABLE_NRF);
	else
		RstBit(CHIP_ENABLE_NRF_OUT,CHIP_ENABLE_NRF);

#endif
#ifdef EDU_CIAA

	Chip_GPIO_SetPinState( LPC_GPIO_PORT, CHIP_ENABLE_GPIO, CHIP_ENABLE_PIN, Value);

#endif

}



/*****************************************************************************************
 * Funcion getAddressMatch
 *****************************************************************************************/
bool getAddressMatch(void)  {
	bool Ret;
	uint8_t Pins;


#ifdef MSP430

	Pins = ADDRESS_MATCH_IN;
	if((Pins & ADDRESS_MATCH) != 0)
		Ret = true;
	else
		Ret = false;

#endif
#ifdef EDU_CIAA

	Ret = Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, ADDRESS_MATCH_GPIO, ADDRESS_MATCH_PIN );

#endif

	return Ret;
}


/*****************************************************************************************
 * Funcion getDataReady
 *****************************************************************************************/
bool getDataReady(void)  {
	bool Ret;
	uint8_t Pins;


#ifdef MSP430

	Pins = DATA_READY_IN;
	if((Pins & DATA_READY) != 0)
		Ret = true;
	else
		Ret = false;

#endif
#ifdef EDU_CIAA

	Ret = Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, DATA_READY_GPIO, DATA_READY_PIN );

#endif

	return Ret;
}


/*****************************************************************************************
 * Funcion getCarrierDetect
 *****************************************************************************************/
bool getCarrierDetect(void)  {
	bool Ret = false;
	uint8_t Pins;

#ifdef MSP430

	Pins = CARRIER_DETECT_IN;
	if((Pins & CARRIER_DETECT) != 0)
		Ret = true;
	else
		Ret = false;

#endif
#ifdef EDU_CIAA

	Ret = Chip_GPIO_ReadPortBit( LPC_GPIO_PORT, CARRIER_DETECT_GPIO, CARRIER_DETECT_PIN );

#endif

	return Ret;
}


/*----------------------------------------------------------------------------------------

		LA PROXIMA SECCION CONTIENE LAS FUNCIONES ACCESIBLES POR MODULOS EXTERNOS

----------------------------------------------------------------------------------------*/


/*****************************************************************************************
 * Funcion getAddressMatch_FromIRQ
 *****************************************************************************************/
bool getAddressMatch_FromIRQ(void)  {

	return g_nRF905_Config.AM;

	//TODO: Si es necesario, codificar aqui para HW limitado

}

/*****************************************************************************************
 * Funcion setAddressMatch_FromIRQ
 *****************************************************************************************/
void setAddressMatch_FromIRQ(bool X)  {

	g_nRF905_Config.AM = X;

	//TODO: Si es necesario, codificar aqui para HW limitado

}


/*****************************************************************************************
 * Funcion getDataReady_FromIRQ
 *****************************************************************************************/
bool getDataReady_FromIRQ(void)  {
	//TODO: Si es necesario, codificar aqui para HW limitado

	return g_nRF905_Config.DR;
}

/*****************************************************************************************
 * Funcion getDataReady_FromIRQ
 *****************************************************************************************/
void setDataReady_FromIRQ(bool X)  {

	g_nRF905_Config.DR = X;

	//TODO: Si es necesario, codificar aqui para HW limitado
}


/*****************************************************************************************
 * Funcion getCarrierDetect_FromIRQ
 *****************************************************************************************/
bool getCarrierDetect_FromIRQ(void)  {

	//TODO: Si es necesario, codificar aqui para HW limitado

	return g_nRF905_Config.CD;
}

/*****************************************************************************************
 * Funcion setCarrierDetect_FromIRQ
 *****************************************************************************************/
void setCarrierDetect_FromIRQ(bool X)  {

	//TODO: Si es necesario, codificar aqui para HW limitado

	g_nRF905_Config.CD = X;

}

/*****************************************************************************************
 * Funcion setSPI_IRQFlag
 *****************************************************************************************/
void setSPI_IRQFlag(void)  {

	//TODO: Si es necesario, codificar aqui para HW limitado

	g_spi_IRQFlag = true;

}



/*
 * Esta funcion devuelve el valor de la variable interna al modulo. Asi la hacemos
 * solo de escritura para las funciones exteriores.
 */
bool nRF905_getStatusReg(uint8_t *status)  {
	bool Ret = false;
	uint8_t Aux;

	//se hace una lectura SPI. Si hay algo en el FIFO esta funcion devuelve true
	//asi que cargamos el valor a la variable pasada por referencia
	if (spi_read(&Aux))  {
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
void nRF905_Init(nRF905 init_struct)  {

	g_nRF905_Config = init_struct;
	setChipEnable(true);				//CS -> High (Deshabilitamos)

	setTRX_ChipEnable(false);			//modulo en powerdown
	setTX_Enable(false);
	setPowerUp(false);

}

/**
 * @brief Esta funcion envia el comando correspondiente a setear la direccion a la cual se transmite.
 *
 * @param Direccion es un valor de 32bits donde seran enviados los datos via RF
 * @return La funcion devuelve @p TRUE si el envio fue exitoso o @p FALSE en caso contrario
 * @warning Las causas de falla normalmente son que no haya lugar en el FIFO.
 */
bool nRF905_setTXAddress(uint32_t Direccion)  {
	bool Ret = false;

	setChipEnable(false);		//Bajamos chip select
	if(!spi_write(C_WRITE_TX_ADDRR))  {	//comando para escribir registros de direccion de TX
		setChipEnable(true);			//excepcion
		return Ret;
	}
	if(!spi_write(Direccion & 0x000000FF))  {		//byte menos significativo
		setChipEnable(true);			//excepcion
		return Ret;
	}
	if(!spi_write((Direccion >> 8) & 0x000000FF)){	//segundo byte
		setChipEnable(true);			//excepcion
		return Ret;
	}
	if(!spi_write((Direccion >> 16) & 0x000000FF))  {	//tercer byte
		setChipEnable(true);			//excepcion
		return Ret;
	}
	if(!spi_write((Direccion >> 24) & 0x000000FF))  {	//Byte mas significativo
		setChipEnable(true);			//excepcion
		return Ret;
	}
	spi_wait();								//espera (solo para tiva)
	spi_flush();							//No nos interesa nada de lo que haya entrado

	setChipEnable(true);					//termino la transaccion
	g_nRF905_Config.DireccionTX = Direccion;	//keep track of TX address
	Ret = true;
	return Ret;
}

bool nRF905_TxPayload_wr(uint8_t *data_tx, uint8_t cant_bytes)  {
	bool Ret = false;
	uint8_t i;

	if (cant_bytes < MAX_TX_RX_PAYLOAD)  {
		setChipEnable(false);		//Bajamos chip select
		if(!spi_write(C_WRITE_TX_PAYLOAD))  {	//comando para escribir registros de direccion de TX
			setChipEnable(true);		//excepcion
			return Ret;
		}

		for (i=0;i<cant_bytes;i++) {
			if(!spi_write(data_tx[i]))  {	//comando para escribir registros de direccion de TX
				setChipEnable(true);		//excepcion
				return Ret;
			}
		}

		spi_wait();								//espera para sincronizar ultimo clock y CS high
		spi_flush();							//No nos interesa nada de lo que haya entrado

		setChipEnable(true);					//termino la transaccion
		Ret = true;
	}


	return Ret;
}

bool nRF905_RxPayload_rd(uint8_t *data_rx, uint8_t cant_bytes)  {
	bool Ret = false;
	uint8_t i;

	if (cant_bytes < MAX_TX_RX_PAYLOAD)  {
		setChipEnable(false);		//Bajamos chip select
		if(!spi_write(C_READ_RX_PAYLOAD))  {	//comando para escribir registros de direccion de TX
			setChipEnable(true);		//excepcion
			return Ret;
		}

		for (i=0;i<cant_bytes;i++) {
			if(!spi_write(data_rx[i]))  {	//comando para escribir registros de direccion de TX
				setChipEnable(true);		//excepcion
				return Ret;
			}
		}

		spi_wait();								//espera para sincronizar ultimo clock y CS high
		spi_flush();							//No nos interesa nada de lo que haya entrado

		setChipEnable(true);					//termino la transaccion
		Ret = true;
	}

	return Ret;
}


bool nRF905_ChanelConfig(void)  {
	bool Ret = false;
	uint8_t cc_MSB,cc_LSB;


	cc_MSB = CONFIG_MASK | (g_nRF905_Config.Potencia << 2) | (g_nRF905_Config.PLL_Freq << 1) |
				((g_nRF905_Config.Canal & 0x100000000) >> 8);
	cc_LSB = g_nRF905_Config.Canal & 0xFF;

	setChipEnable(false);		//Bajamos chip select
	if(!spi_write(cc_MSB))  {	//comando para configurar el canal (el comando esta incluido en CONFIG_MASK)
		setChipEnable(true);		//excepcion
		return Ret;
	}
	if(!spi_write(cc_LSB))  {	//parte baja del comando config channel
		setChipEnable(true);		//excepcion
		return Ret;
	}

	spi_wait();								//espera para sincronizar ultimo clock y CS high
	spi_flush();							//No nos interesa nada de lo que haya entrado

	setChipEnable(true);					//termino la transaccion
	Ret = true;

	return Ret;
}

bool nRF905_WriteConfig(void)  {
	bool Ret = false;
	uint8_t config_reg[CONFIG_REG_LENGTH];
	uint8_t i;

	config_reg[0] = g_nRF905_Config.Canal & 0xFF;
	config_reg[1] = (g_nRF905_Config.Retransmision << 5) | (g_nRF905_Config.Potencia_Rx << 4) |
					(g_nRF905_Config.Potencia << 2) | (g_nRF905_Config.PLL_Freq << 1) |
					((g_nRF905_Config.Canal & 0x100) >> 8);
	config_reg[2] = (g_nRF905_Config.LongTX_Address << 4) | g_nRF905_Config.LongRX_Address;
	config_reg[3] = g_nRF905_Config.LongRX_Payload;
	config_reg[4] = g_nRF905_Config.LongTX_Payload;

	config_reg[5] = g_nRF905_Config.DireccionRX & 0xFF;
	config_reg[6] = (g_nRF905_Config.DireccionRX >> 8) & 0xFF;
	config_reg[7] = (g_nRF905_Config.DireccionRX >> 16) & 0xFF;
	config_reg[8] = (g_nRF905_Config.DireccionRX >> 24) & 0xFF;
	config_reg[9] = (g_nRF905_Config.CRC_Mode << 7) | (g_nRF905_Config.CRC_Enable << 6) |
					(g_nRF905_Config.ClockModulo << 3) | (g_nRF905_Config.ClockOut << 2)|
					g_nRF905_Config.ClockOut_Freq;


	setChipEnable(false);		//Bajamos chip select

	if(!	spi_write(C_WRITE_CONFIG))  {	//comando para escribir en el registro de configuracion
		setChipEnable(true);		//excepcion
		return Ret;
	}

	for(i=0;i<CONFIG_REG_LENGTH;i++)  {
		if(!spi_write(config_reg[i]))  {	//comando para escribir en el registro de configuracion
			setChipEnable(true);		//excepcion
			return Ret;
		}
	}

	spi_wait();								//espera para sincronizar ultimo clock y CS high
	spi_flush();							//No nos interesa nada de lo que haya entrado

	setChipEnable(true);					//termino la transaccion
	Ret = true;

	return Ret;
}

//*********************************************************************************************



/*=============================================================================================
 * 				FUNCIONES BASICAS PARA TRANSMISION Y RECEPCION DE DATOS VIA RF
 *
 * 	Estas funciones son extensiones de las funciones basicas de la HAL
 * 	No son parte de la HAL, sino que se construyen en base a estas ultimas. En nivel de capas
 * 	estan por sobre la HAL, pero a su vez, tanto las funciones HAL utilizadas, como las de
 * 	Tx/Rx implementadas, son accesibles por el programador.
 =============================================================================================*/

void nRF905_PowerMode(ePowerMode_t X)  {

	switch(X)  {

	case ePowerDown:
		setTRX_ChipEnable(false);
		setTX_Enable(false);
		setPowerUp(false);
		break;

	case eStandBy:
		setTRX_ChipEnable(false);
		setTX_Enable(false);
		setPowerUp(true);

		//esperar 3ms a STND_BY (PWR_DOWN -> STND_BY 3ms)
		g_nRF905_Config.Delay_ms(3);
		break;

	case eRadioEnabled:
		setTRX_ChipEnable(true);
		setPowerUp(true);
		break;

	default:
		;
	}
}
/**
 * Esta funcion envia datos via RF segun el payload que se pase como argumento
 * el manejo de los power modes debe ser hecho desde fuera, por el protocolo
 * esto solamente para agregar generalidad
 *
 * @warning La funcion debe ser llamada con el modulo en STAND_BY, sino la transmision no sera exitosa
 */
void nRF905_RF_TxData(uint32_t address,uint8_t *Payload,uint8_t cant_Bytes,bool keep_radio_on)  {

	// Salimos de PWR_DOWN
	setTRX_ChipEnable(false);
	setPowerUp(true);

	//TX_EN = 1 --> Transmision
	setTX_Enable(true);

	//esperar 3ms a STND_BY (PWR_DOWN -> STND_BY 3ms)
	g_nRF905_Config.Delay_ms(3);

	//Cargar payload y TX address
	nRF905_setTXAddress(address);
	nRF905_TxPayload_wr(Payload,cant_Bytes);

	//TRX_CE = 1 (Radio encendida)
	setTRX_ChipEnable(true);

	if(!keep_radio_on)  {
		//esperar 2ms (son aproximados, puede ser 1.1 a 2 ms)
		g_nRF905_Config.Delay_ms(2);

		//TRX_CE = 0 (Radio apagada)
		setTRX_ChipEnable(false);
	}

	//esperar tpreamble + (total Bites / 50kbps) ==> 200us + (32(address) + 32*8(payload) + 16(CRC)) / 50e3
	//	* --> esperar 6.28ms (redondeamos a 7)
	g_nRF905_Config.Delay_ms(7);
}

/**
 * Esta funcion recibe datos via RF segun el paquete que hayamos conformado antes
 *
 * @param *Paquete es en realidad un paquete de recepcion del cual ya se conoce el tipo
 * 			solamente se carga el payload, pero asi podemos tener trazabilidad del tipo
 * 			de datos que se esta recibiendo
 * @return Devuelve true si los datos recibidos tienen un CRC correcto y false si los datos estan
 * 			corruptos
 */
eRxStatus_t nRF905_RF_RxData(uint8_t *Payload,uint8_t cant_Bytes,bool keep_radio_on)  {
	eRxStatus_t Ret = eNoAM;
	uint8_t timeout;

	// Salimos de PWR_DOWN
	//setTRX_ChipEnable(false);
	setPowerUp(true);

	//TX_EN = 0 --> recepcion
	setTX_Enable(false);

	//esperar 3ms a STND_BY (PWR_DOWN -> STND_BY 3ms)
	g_nRF905_Config.Delay_ms(3);

	//escuchamos por 21 ms	(equivale a 3 transmisiones completas)
	setTRX_ChipEnable(true);

	timeout = TIME_OUT_AM;
	while (!getAddressMatch() &&  timeout != 0)  {
		timeout--;
		g_nRF905_Config.Delay_ms(1);
	}

	//si salimos por un timeout, retornamos aqui (excepcion)
	if (timeout == 0)  {
		setTRX_ChipEnable(false);	//apagamos la radio, excepcion
		return Ret;
	}

	//Cuando se termine de recibir los datos tenemos dos opciones
	//o AddressMatch se pone a cero, por un CRC erroneo
	//o DataReady se pone en alto por un CRC correcto
	while (getAddressMatch() &&  !getDataReady())  {
		g_nRF905_Config.Delay_ms(1);
	}

	//si salimos porque AddressMatch se puso a cero, retornamos aqui (excepcion)
	if (!getDataReady())  {
		setTRX_ChipEnable(false);	//apagamos la radio, excepcion
		Ret = eCRCFail;
		return Ret;
	}

	if(!keep_radio_on)  {
		setTRX_ChipEnable(false);	//apagamos la radio
	}

	//Datos recibidos con CRC correcto, a sacarlos del payload
	nRF905_RxPayload_rd(Payload,cant_Bytes);
	Ret = eDataReady;

	return Ret;
}












