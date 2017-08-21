/*
 * nRF905_Defs.h
 *
 *  Created on: 19/08/2017
 *      Author: Gonza
 */

#ifndef NRF905_H_
#define NRF905_H_

#include <stdint.h>
#include <stdbool.h>

extern nRF905 g_nRF905_Config;

/**
 * Definiciones Dependiendo de la plataforma de HW utilizada.
 * La definicion debe ser hecha en cada compilacion en la seccion de
 * PREDEFINED SYMBOLS. Equivale a --define HW_PLATFORM para compilar en linea de comandos
 */

#ifdef TIVA_C
#define TX_EN_BASE
#define TX_EN_GPIO

#define TRX_CE_BASE
#define TRX_CE_GPIO

#define PWR_UP_BASE
#define PWR_UP_GPIO
#endif

#ifdef MSP430
#endif

#ifdef EDU_CIAA
#endif


/**
 * Definicion de estructura de datos para contener configuracion del driver
 * Las definiciones de CD, AM, y DR son para soportar la utilizacion de interrupciones
 */

struct _nRF905 {
	uint8_t Canal;				//Canal en el que esta trabajando el modulo
	uint8_t Potencia;			//potencia de transmision
	uint8_t PLL_Freq;			//Frecuencia de PLL
	uint8_t Modo_Operacion;		//Modo de operacion refiere a seccion 8.2 del manual
	bool Retransmision;			//retransmision true or false
	bool ClockOut;				//habilitacion de clockout
	uint8_t ClockModulo;		//frecuencia de cristal utilizado para alimentar el PLL
	uint32_t DireccionTX;		//direccion a la cual se quiere transmitir
	uint32_t DireccionRX;		//direccion en la cual se recibiran datos
	uint8_t LongRX_Payload;		//largo del payload RX
	uint8_t LongTX_Payload;		//largo del payload TX

	bool CD;					//Flag Carrier Detect para interrupciones
	bool AM;					//Flag Address Match para interrupciones
	bool DR;					//Flag Data Ready para interrupciones
						//Este set de FLAGS solo sera actualizado dentro de
						//IRQ handlers (tambien la limpieza de las mismas).
						//En los casos que la limpieza no sea posible, por limitaciones
						//de HW, es responsabilidad del programador la limpieza de
						//estos FLAGS
};

typedef struct _nRF905 nRF905;

#endif /* NRF905_H_ */
