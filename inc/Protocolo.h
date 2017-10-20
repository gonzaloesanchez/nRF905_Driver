/*
 * Protocolo.h
 *
 *  Created on: 19 oct. 2017
 *      Author: gonza
 */

#ifndef NRF905_DRIVER_INC_PROTOCOLO_H_
#define NRF905_DRIVER_INC_PROTOCOLO_H_


enum _eComm {eCommFail=-1,eCommNAck=0,eCommAck=1};
enum _eTipo {eTypeACK=0,eTypePeso=0x0D/*TODO: Completar tipos de datos */};

typedef enum _eComm eComm_t;
typedef enum _eTipo eTipo_t;

struct _sPacket  {
	uint8_t sizeofPacket;
	eTipo_t Tipo;
	uint8_t Payload[MAX_TX_RX_PAYLOAD-2];
};

typedef struct _sPacket sPacket_t;


#endif /* NRF905_DRIVER_INC_PROTOCOLO_H_ */
