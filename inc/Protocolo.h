/*
 * Protocolo.h
 *
 *  Created on: 19 oct. 2017
 *      Author: gonza
 */

#ifndef NRF905_DRIVER_INC_PROTOCOLO_H_
#define NRF905_DRIVER_INC_PROTOCOLO_H_

#include "nRF905.h"
#include <string.h>

#define MAX_PACKET_PAYLOAD	26

enum _eCommStatus {eCommFail=-1,eCommNAck=0,eCommAck=1};
enum _eTipoComm {eTipoACK = 0,
				 eTipoNACK = 0xFF,
				 eTipoPeso = 0x0D,
				 eTipoPesoReq = 0x1D,
				 eTipoLogReq = 0x01,
				 eTipoConfig = 0x02,
				 eTipoEstadoBatReq = 0xEB,
				 eTipoEstadoBat = 0xEC,
				 eTipoMensajeReq = 0x03,
				 eTipoCheckCommReq = 0xCC,
				 eTipoCheckCommDev = 0xCD,
				 eTipoCheckCommDevF = 0xCF
				};

typedef enum _eCommStatus eCommStatus_t;
typedef enum _eTipoComm eTipoComm_t;

struct _sPacket  {
	eTipoComm_t Tipo;
	uint32_t Address;
	uint8_t sizeofData;
	uint8_t Payload[MAX_PACKET_PAYLOAD];

};

typedef struct _sPacket sPacket_t;

bool Protocol_Rx(sPacket_t *Paquete,bool keep_radio_on);
void Protocol_Tx(uint32_t AddresTo,sPacket_t *Paquete,bool keep_radio_on);
sPacket_t Protocol_PacketForm(uint32_t AddressFrom,eTipoComm_t Tipo,void* Data);
void Protocol_CheckMedio(void);
void Protocol_Init(void(*delay_func)(uint32_t x));


#endif /* NRF905_DRIVER_INC_PROTOCOLO_H_ */
