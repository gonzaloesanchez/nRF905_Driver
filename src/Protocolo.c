/*
 * Protocolo.c
 *
 *  Created on: 19 oct. 2017
 *      Author: gonza
 */

//TODO: ordenar esto, es un quilombo

#include "Protocolo.h"

static void (*External_delay)(uint32_t delay_ms);



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

	External_delay(g_rand[contador]);		//funcion external delay esta implementada fuera de este modulo

	contador++;					//esta porcion de codigo incrementa contador en forma
	contador %= RAND_LENGTH;	//ciclica y evita que contador salga del limite de RAND_LENGTH

}


/*=============================================================================================
 * 				FUNCIONES BASICAS PARA TRANSMISION Y RECEPCION DE DATOS VIA RF
 *
 * 	Estas funciones son propias de esta implementacion, dadas para el protocolo desarrollado
 * 	No son parte de la HAL, sino que se construyen en base a estas ultimas. En nivel de capas
 * 	estan por sobre la HAL, pero a su vez, tanto las funciones HAL utilizadas, como las de
 * 	protocolo implementadas, son accesibles por el programador.
 =============================================================================================*/


/**
 * Esta funcion solamente pasa el puntero de la funcion delay externa hacia un puntero interno
 * para uso encapsulado. Nos da la posibilidad de definir la funcion delay como se desee
 */
void Protocol_Init(void(*delay_func)(uint32_t x))  {
	External_delay = delay_func;		//esta es la funcion de delay que se toma desde fuera
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
void Protocol_CheckMedio(void)  {

	while (getCarrierDetect())
		espera_aleatoria();
}

sPacket_t Protocol_PacketForm(uint32_t AddressFrom,eTipoComm_t Tipo,void* Data)  {
	sPacket_t Packet;
	uint8_t i;

	Packet.Tipo = Tipo;
	Packet.Address = AddressFrom;


	switch (Tipo)  {

	case eTipoACK:
	case eTipoNACK:
	case eTipoPesoReq:
	case eTipoEstadoBatReq:
	case eTipoMensajeReq:
	case eTipoCheckCommReq:
	case eTipoCheckCommDev:
		Packet.sizeofData = 0;
		break;


	case eTipoPeso:
	case eTipoConfig:
		Packet.sizeofData = sizeof(uint32_t);
		memcpy(Packet.Payload,Data,Packet.sizeofData);
		break;


	case eTipoLogReq:
		i = 0;

		//este while nos asegura que la cadena enviada nunca sera mayor al lugar
		//que se tiene para almacenarla, a lo sumo se cortara
		while((((char*)Data)[i] != NULL) && (i < MAX_PACKET_PAYLOAD))
			i++;

		if(i == MAX_PACKET_PAYLOAD-1)	//en el caso de que la cadena sea demasiado larga
			((char*)Data)[i] = NULL;				//la cortamos en el ultimo byte disponible

		Packet.sizeofData = i;
		memcpy(Packet.Payload,Data,Packet.sizeofData);
		break;


	case eTipoEstadoBat:
		Packet.sizeofData = sizeof(uint16_t);
		memcpy(Packet.Payload,Data,Packet.sizeofData);
		break;


	case eTipoCheckCommDevF:
		Packet.sizeofData = sizeof(uint8_t);
		memcpy(Packet.Payload,Data,Packet.sizeofData);
		break;

	default:
		;
	}

	return Packet;

}

/**
 * Esta funcion arma el paquete del protocolo y lo envia mediante la HAL del nRF905
 * implementada.
 */
void Protocol_Tx(uint32_t AddresTo,sPacket_t *Paquete,bool retrans)  {
	uint8_t Buffer[MAX_TX_RX_PAYLOAD];
	uint8_t pointer = 0;

	Buffer[pointer] = (uint8_t)Paquete->Tipo;		//primer byte = tipo de paquete
	pointer += sizeof(uint8_t);

	memcpy(Buffer+pointer,&(Paquete->Address),sizeof(uint32_t));	//siguientes 4 bytes = direccion
	pointer += sizeof(uint32_t);

	Buffer[pointer] = Paquete->sizeofData;					//siguiente byte = cantidad de bytes en data

	if(Paquete->sizeofData != 0)  {
		pointer += sizeof(uint8_t);							//si es != 0 cargamos data
		memcpy((Buffer+pointer),Paquete->Payload,Paquete->sizeofData);
	}

	nRF905_RF_TxData(AddresTo,Buffer,MAX_TX_RX_PAYLOAD,retrans);		//envio de datos RAW
}

/**
 * Esta funcion se encarga de recibir los datos desde un dispositivo remoto
 */
bool Protocol_Rx(sPacket_t *Paquete,bool retrans)  {
	uint8_t Buffer[MAX_TX_RX_PAYLOAD];
	uint8_t pointer = 0;
	eRxStatus_t rxStatus;
	bool Ret = false;

	rxStatus = nRF905_RF_RxData(Buffer,MAX_TX_RX_PAYLOAD,retrans);		//recepcion de RAW data

	if (rxStatus == eDataReady)  {
		Paquete->Tipo = (eTipoComm_t)Buffer[pointer];					//si la recepcion fue exitosa
		pointer += sizeof(uint8_t);										//extraemos el tipo de datos

		memcpy(&(Paquete->Address),Buffer+pointer,sizeof(uint32_t));
		pointer += sizeof(Paquete->Address);								//extraemos quien lo esta enviando

		Paquete->sizeofData = Buffer[pointer];

		if(Paquete->sizeofData != 0)  {
			pointer += sizeof(uint8_t);						//extraemos data
			memcpy(Paquete->Payload,(Buffer+pointer),Paquete->sizeofData);
		}

		Ret = true;
	}

	return Ret;
}











