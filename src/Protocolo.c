/*
 * Protocolo.c
 *
 *  Created on: 19 oct. 2017
 *      Author: gonza
 */

//TODO: ordenar esto, es un quilombo

static uint16_t g_Ticks_ms;		//esta variable es de apoyo para calcular los tiempos
								//de espera requeridos por distintas funciones del protocolo
								//a implementar

//definiciones de vectores "Aleatorios" para los retardos aleatorios necesarios en el protocolo
//compilacion dependiente del dispositivo. Esto es para ahorrar tiempo de ejecucion y memoria
#ifdef DEV_1
static const uint8_t g_rand[RAND_LENGTH] = {14,12,67,13,10,34,13,60,25,59};
#endif
#ifdef DEV_2
static const uint8_t g_rand[RAND_LENGTH] = {69,8,19,7,44,66,61,64,48,20};
#endif
#ifdef DEV_3
static const uint8_t g_rand[RAND_LENGTH] = {55,58,40,45,51,66,62,10,12,43};
#endif
#ifdef COORD
static const uint8_t g_rand[RAND_LENGTH] = {51,48,25,61,44,34,26,35,59,61};
#endif




static void espera_aleatoria(void)  {
	static uint8_t contador = 0;
	uint16_t ticks_inicial;

	ticks_inicial = g_Ticks_ms;

	while ((g_Ticks_ms-ticks_inicial) < g_rand[contador])
		wait_for_interrupt();

	contador++;					//esta porcion de codigo incrementa contador en forma
	contador %= RAND_LENGTH;	//ciclica y evita que contador salga del limite de RAND_LENGTH

}




/**
 * Esta funcion es "bloqueante", queda en espera hasta que se libere el medio
 * La implementacion es la siguiente: se hace un polling del pin CarrierDetect
 * mediante un bucle con esperas aleatorias entre pasadas hasta que Carrier Detect = LOW
 *
 * @note Es bloqueante con respecto al programa, pero libera el procesador esperando
 * 		 interrupciones para la espera aleatoria
 *
 * @note Tiempo en el aire de un paquete completo desde la activacion de Tx es ~6.93[ms]
 */
void nRF905_CheckMedio(void)  {

	while (getCarrierDetect())
		espera_aleatoria();
}

/**
 * Esta funcion debe ser llamada desde la interrupcion de SysTick (o timer) con periodo 1[ms]
 * si accion = true -> incrementamos el contador. si accion = false -> el contador se pone a cero
 * Devuelve el valor de la variable Ticks
 */
void nRF905_TickIncrease(void)  {
	g_Ticks_ms++;
}

void nRF905_TickClear(void)  {
	g_Ticks_ms = 0;
}

uint16_t nRF905_TickGet(void)  {
	return g_Ticks_ms;
}


/*****************************************************************************************
 * Funcion wait_for_interrupt es en realidad un encapsulamiento de una instruccion WFI
 * o de LPM
 *****************************************************************************************/
static void wait_for_interrupt(void)  {


#ifdef EDU_CIAA
	__WFI();
#endif


#ifdef MSP430
	LPM2;			//macro para entrar en LPM2.
					//checkear que el SMCLK siga funcionando en este modo
					//recordar lpm_exit_on_return() para el timer que implemente el tick
#endif
}





/*=============================================================================================
 * 				FUNCIONES BASICAS PARA TRANSMISION Y RECEPCION DE DATOS VIA RF
 *
 * 	Estas funciones son propias de esta implementacion, dadas para el protocolo desarrollado
 * 	No son parte de la HAL, sino que se construyen en base a estas ultimas. En nivel de capas
 * 	estan por sobre la HAL, pero a su vez, tanto las funciones HAL utilizadas, como las de
 * 	protocolo implementadas, son accesibles por el programador.
 =============================================================================================*/

static void espera_aleatoria(void)  {
	static uint8_t contador = 0;
	uint16_t ticks_inicial;

	ticks_inicial = g_Ticks_ms;

	while ((g_Ticks_ms-ticks_inicial) < g_rand[contador])
		wait_for_interrupt();

	contador++;					//esta porcion de codigo incrementa contador en forma
	contador %= RAND_LENGTH;	//ciclica y evita que contador salga del limite de RAND_LENGTH

}


/**
 * Esta funcion debe ser llamada desde la interrupcion de SysTick (o timer) con periodo 1[ms]
 * si accion = true -> incrementamos el contador. si accion = false -> el contador se pone a cero
 * Devuelve el valor de la variable Ticks
 */
void nRF905_TickIncrease(void)  {
	g_Ticks_ms++;
}

void nRF905_TickClear(void)  {
	g_Ticks_ms = 0;
}

uint16_t nRF905_TickGet(void)  {
	return g_Ticks_ms;
}

/**
 * Esta funcion es "bloqueante", queda en espera hasta que se libere el medio
 * La implementacion es la siguiente: se hace un polling del pin CarrierDetect
 * mediante un bucle con esperas aleatorias entre pasadas hasta que Carrier Detect = LOW
 *
 * @note Es bloqueante con respecto al programa, pero libera el procesador esperando
 * 		 interrupciones para la espera aleatoria
 *
 * @note Tiempo en el aire de un paquete completo desde la activacion de Tx es ~6.93[ms]
 */
void nRF905_CheckMedio(void)  {

	while (getCarrierDetect())
		espera_aleatoria();
}


/**
 * Esta funcion envia datos via RF segun el paquete que hayamos conformado antes
 *
 * @param Retrans Indica si hay que intentar retransmitir los datos. Si no se pide
 * 				  retransmision, la funcion devuelve eCommNAck (no tiene significado)
 */
void nRF905_RF_TxData(sPacket_t *Paquete)  {

	/* salir de POWER_DOWN mode
	 * esperar 3ms a STND_BY (PWR_DOWN = 1, TX_EN = 1)
	* Cargar payload y TX address
	* TRX_CE = 1
	* esperar 1ms
	* TRX_CE = 1
	* esperar tpreamble + (total Bites / 50kbps) ==> 200us + (32(address) + 32*8(payload) + 16(CRC)) / 50e3
	* --> esperar 6.28ms (redondeamos a 7)  */
}

/**
 * Esta funcion recibe datos via RF segun el paquete que hayamos conformado antes
 *
 * @param *Paquete es en realidad un paquete de recepcion del cual ya se conoce el tipo
 * 			solamente se carga el payload, pero asi podemos tener trazabilidad del tipo
 * 			de datos que se esta recibiendo
 */
eComm_t nRF905_RF_RxData(sPacket_t *Paquete)  {

}

