/**
 * @file nRF905.h
 * @brief Archivo con definiciones necesarias para el driver de nRF905
 * @date 19/08/2017
 * @author Gonza
 * @see http://www.nordicsemi.com/eng/nordic/download_resource/8075/1/73282575/2452
 */

#ifndef NRF905_H_
#define NRF905_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Definiciones generales para el proyecto.
 * Aqui se define la frecuencia de SPI y ancho en bits de los paquetes que son enviados por
 * el bus SPI. Ademas de definirse algunas constantes de uso como ser @c ON y @c OFF
 */

#define SPI_FREC		1e6		/**<Constante numerica para 1[MHz]*/
#define	SPI_DATAWIDTH	8		/**<Longitud en bits de la transferencia*/

#define ON				0xFF
#define OFF				0x00

#define C_WRITE_TX_ADD	0x22	//Comando SPI para escribir la direccion donde transmitir


/**
 * Definiciones Dependiendo de la plataforma de HW utilizada.
 * La definicion debe ser hecha en cada compilacion en la seccion de
 * PREDEFINED SYMBOLS. Equivale a --define HW_PLATFORM para compilar en linea de comandos
 */

#ifdef TIVA_C

/**
 * Includes especificos para TivaC
 */
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"


/**
 * Definiciones de hardware para pines
 */
#define TX_EN_BASE				GPIO_PORTB_BASE
#define TX_EN_GPIO				GPIO_PIN_5

#define TRX_CE_BASE				GPIO_PORTB_BASE
#define TRX_CE_GPIO				GPIO_PIN_0

#define PWR_UP_BASE				GPIO_PORTB_BASE
#define PWR_UP_GPIO				GPIO_PIN_1

#define ADDRESS_MATCH_BASE		GPIO_PORTB_BASE
#define ADDRESS_MATCH_GPIO		GPIO_PIN_2

#define DATA_READY_BASE			GPIO_PORTB_BASE
#define DATA_READY_GPIO			GPIO_PIN_6

#define CARRIER_DETECT_BASE		GPIO_PORTB_BASE
#define CARRIER_DETECT_GPIO		GPIO_PIN_7

#define CHIP_ENABLE_BASE		GPIO_PORTA_BASE
#define CHIP_ENABLE_GPIO		GPIO_PIN_3
/**
 * Definiciones de hardware para SPI
 */

#define SPI_BASE				SSI0_BASE

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

struct _spi_flags  {
	bool irqTX;
	bool irqRX;
};

typedef struct _nRF905 nRF905;
typedef struct _spi_flags spi_flags;

extern nRF905 g_nRF905_Config;

void nRF905_setTXFlag(void);
bool nRF905_setTXAddress(uint32_t Direccion);

void escribir(void);
void leer(void);

#endif /* NRF905_H_ */
