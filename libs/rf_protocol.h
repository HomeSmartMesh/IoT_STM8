/** @file rf_protocol.h
 *
 * @author Wassim FILALI
 *
 * @compiler IAR STM8
 *
 *
 * $Date: 29.10.2016 - creation
 * $Date: 18.11.2017 - update to Mesh Protocol 2.0
 * $Revision: 2
 *
*/

#include "commonTypes.h"


#define rfi_size    0x00
#define rfi_ctr     0x01
#define rfi_pid     0x02
#define rfi_src     0x03
#define rfi_dst     0x04
#define rfi_broadcast_header_size       0x04
#define rfi_broadcast_payload_offset    0x04
#define rfi_p2p_header_size             0x05
#define rfi_p2p_payload_offset          0x05

//bit 7
#define rf_ctr_Broadcast   0x80
#define rf_ctr_Peer2Peer   0x00
//bit 6
#define rf_ctr_Msg_Ack     0x40
#define rf_ctr_ReqResp     0x00
//bit 5
#define rf_ctr_Message     0x20
#define rf_ctr_Acknowledge 0x00
#define rf_ctr_Request     0x20
#define rf_ctr_Response    0x00
//bit 4
#define rf_ctr_Send_Ack    0x10
#define rf_ctr_No_Ack      0x00
//bits 3-0
#define rf_ctr_ttl_mask    0x0F
#define rf_ctr_ttl_clear   0xF0


#define rf_pid_ping         0x01
#define rf_pid_switchChan   0x03
#define rf_pid_reset		0x04
#define rf_pid_alive		0x05
#define rf_pid_temperature	0x08
#define rf_pid_light		0x07
#define rf_pid_bme280		0x0A
#define rf_pid_rgb 		    0x0B
#define rf_pid_magnet		0x0C


// Alive RF ping
void rf_get_tx_alive_3B(BYTE NodeId, BYTE* tx_data);
void rx_alive(BYTE src_NodeId);//Rx 3 Bytes

// Reset
void rf_get_tx_reset_3B(BYTE NodeId, BYTE* tx_data);
void rx_reset(BYTE src_NodeId);

//should move to max44009 lib
void rx_light(BYTE src_NodeId,BYTE *rxPayload,BYTE rx_PayloadSize);

//should move to magnet lib
void rx_magnet(BYTE src_NodeId,BYTE *rxPayload,BYTE rx_PayloadSize);

