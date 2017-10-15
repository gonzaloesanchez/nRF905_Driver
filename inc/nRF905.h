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
 * LA SIGUIENTE DEFINICION ES LA QUE DETERMINA PARA QUE PLACA SE VA A COMPILAR ESTO
 */
//#define EDU_CIAA

/**
 * @brief Definiciones generales para el proyecto.
 * Aqui se define la frecuencia de SPI y ancho en bits de los paquetes que son enviados por
 * el bus SPI. Ademas de definirse algunas constantes de uso como ser @c ON y @c OFF
 */

#define C_WRITE_TX_ADDRR		0x22	//Comando SPI para escribir la direccion donde transmitir
#define C_WRITE_TX_PAYLOAD		0x20	//Comando SPI para escribir los datos a transmitir
#define C_READ_RX_PAYLOAD		0x24	//Comando SPI para leer los datos recibidos


#define CONFIG_MASK				0x80	//mascara para el comando SPI especial ChannelConfig
#define MAX_TX_RX_PAYLOAD		32		//ningun payload puede ser mayor a 32 bytes (dentro de esos 32bytes se
										//implementa el protocolo que uno desee
#define CONFIG_REG_LENGTH		10		//longitud en bytes del registro de configuracion
#define HFREQ_PLL_433			0		//bit para especificar al PLL en que banda trabajar (433[MHz])
#define HFREQ_PLL_868_915		1		//868[MHz] o 915[MHz]
#define DEFAULT_CHANNEL			0x006B	//esto se utiliza en la inicializacion, corresponde a 433.1[MHz]

#define XOF_4MHZ				0		//Seteos necesarios para indicar al integrado el cristal que
#define XOF_8MHZ				1		//utiliza como base para el PLL
#define XOF_12MHZ				2
#define XOF_16MHZ				3
#define XOF_20MHZ				4

#define CLKOUT_4MHZ				0
#define CLKOUT_2MHZ				1
#define CLKOUT_1MHZ				2
#define CLKOUT_500KHZ			3

#define PA_PWR_MINUS_10DBM		0
#define PA_PWR_MINUS_2DBM		1
#define PA_PWR_PLUS_6DBM		2
#define PA_PWR_PLUS_10DBM		3

#define CRC16_MODE				1
#define CRC8_MODE				0


/**
 * Definiciones Dependiendo de la plataforma de HW utilizada.
 * La definicion debe ser hecha en cada compilacion en la seccion de
 * PREDEFINED SYMBOLS. Equivale a --define HW_PLATFORM para compilar en linea de comandos
 */

#ifdef MSP430

//algunas macros para facilitar la lectura de las operaciones de bits
#define SetBit(Dir, Bit) Dir |= (Bit)
#define RstBit(Dir, Bit) Dir &= ~(Bit)
#define ToggleBit(Dir, Bit) Dir ^= (Bit)

//P1.4 = CHIP_ENABLE_NRF
#define CHIP_ENABLE_NRF			BIT4
#define CHIP_ENABLE_NRF_DIR	P1DIR
#define CHIP_ENABLE_NRF_OUT	P1OUT

//P2.0 = PWR_UP
#define PWR_UP					BIT0
#define PWR_UP_DIR			P2DIR
#define PWR_UP_OUT			P2OUT

//P2.1 = TRX_CE
#define TRX_CE					BIT1
#define TRX_CE_DIR			P2DIR
#define TRX_CE_OUT			P2OUT

//P2.2 = TX_EN
#define TX_EN					BIT2
#define TX_EN_DIR				P2DIR
#define TX_EN_OUT				P2OUT

//P2.3 = DATA_READY
#define DATA_READY				BIT3
#define DATA_READY_DIR			P2DIR
#define DATA_READY_IN			P2IN

//P2.4 = ADDRESS_MATCH
#define ADDRESS_MATCH				BIT4
#define ADDRESS_MATCH_DIR			P2DIR
#define ADDRESS_MATCH_IN			P2IN

//P2.5 = CARRIER_DETECT
#define CARRIER_DETECT				BIT5
#define CARRIER_DETECT_DIR			P2DIR
#define CARRIER_DETECT_IN			P2IN

#endif

#ifdef EDU_CIAA
/**
 * Includes especificos para Edu CIAA
 */
#include "chip.h"

#define	TCCH_NRF905			1020

/**
 * Definiciones de hardware para pines
 */

//GPIO0 = PWR_UP
#define PWR_UP_P    		6
#define PWR_UP_P_   		1
#define PWR_UP_GPIO 		3
#define PWR_UP_PIN  		0

//GPIO1 = ADDRESS_MATCH
#define ADDRESS_MATCH_P    	6
#define ADDRESS_MATCH_P_   	4
#define ADDRESS_MATCH_GPIO 	3
#define ADDRESS_MATCH_PIN  	3

//GPIO2 = DATA_READY
#define DATA_READY_P    	6
#define DATA_READY_P_   	5
#define DATA_READY_GPIO 	3
#define DATA_READY_PIN  	4

//GPIO3 = CARRIER_DETECT
#define CARRIER_DETECT_P    6
#define CARRIER_DETECT_P_  	7
#define CARRIER_DETECT_GPIO 5
#define CARRIER_DETECT_PIN 	15

//GPIO4 = CHIP_ENABLE
#define CHIP_ENABLE_P    	6
#define CHIP_ENABLE_P_  	8
#define CHIP_ENABLE_GPIO 	5
#define CHIP_ENABLE_PIN 	16

//GPIO5 = TX_EN
#define TX_EN_P    			6
#define TX_EN_P_  			9
#define TX_EN_GPIO 			3
#define TX_EN_PIN 			5


//GPIO6 = TRX_CE
#define TRX_CE_P    		6
#define TRX_CE_P_  			10
#define TRX_CE_GPIO 		3
#define TRX_CE_PIN 			6


#endif


/**
 * Definicion de estructura de datos para contener configuracion del driver
 * Las definiciones de CD, AM, y DR son para soportar la utilizacion de interrupciones
 */

struct _nRF905 {
	uint16_t Canal;				//Canal en el que esta trabajando el modulo
	uint8_t Potencia;			//potencia de transmision
	bool PLL_Freq;			//Frecuencia de PLL
	uint8_t Modo_Operacion;		//Modo de operacion refiere a seccion 8.2 del manual
	bool Retransmision;			//retransmision true or false
	bool ClockOut;				//habilitacion de clockout
	uint8_t ClockOut_Freq;		//Frecuencia de salida del reloj producido por el modulo
	bool Potencia_Rx;			//habilitacion de reduccion de potencia en recepcion
	uint8_t ClockModulo;		//frecuencia de cristal utilizado para alimentar el PLL
	uint32_t DireccionTX;		//direccion a la cual se quiere transmitir
	uint32_t DireccionRX;		//direccion en la cual se recibiran datos
	uint8_t LongRX_Payload;		//largo del payload RX
	uint8_t LongTX_Payload;		//largo del payload TX
	uint8_t LongRX_Address;		//cantidad de bytes de longitud de la direccion de recepcion
	uint8_t LongTX_Address;		//cantidad de bytes de longitud de la direccion de transmision
	bool CRC_Mode;				//Booleano para describir el modo crc (1 => CRC16; 0 => CRC8)
	bool CRC_Enable;			//Habilitacion del checkeo CRC

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

//extern nRF905 g_nRF905_Config;

bool getAddressMatch_FromIRQ(void);
void setAddressMatch_FromIRQ(bool X);
bool getDataReady_FromIRQ(void);
void setDataReady_FromIRQ(bool X);
bool getCarrierDetect_FromIRQ(void);
void setCarrierDetect_FromIRQ(bool X);

void nRF905_Init(void);
void nRF905_setTXFlag(void);
bool nRF905_setTXAddress(uint32_t Direccion);
bool nRF905_TxPayload_wr(uint8_t *data_tx, uint8_t cant_bytes);
bool nRF905_RxPayload_rd(uint8_t *data_rx, uint8_t cant_bytes);
bool nRF905_ChanelConfig(void);
bool nRF905_WriteConfig(void);


#endif /* NRF905_H_ */
