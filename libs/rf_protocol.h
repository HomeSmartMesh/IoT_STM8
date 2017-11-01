/** @file rf_protocol.h
 *
 * @author Wassim FILALI
 *
 * @compiler IAR STM8
 *
 *
 * $Date: 29.10.2016 - creation
 * $Revision: 1 
 *
*/

#include "commonTypes.h"

//The RF Module addresses might not fit the final TargetId 
//Thus Source and Dest would still be required in addition to the RF address
//The Schockburst Ack is a point to point Acknowledge

//The source Node Id is needed most of the time
//The target Node Id is also needed most of the time
//0xFF should be kept as Broadcast and No Address

//protocol ids for the first identification byte

//[7] 1: Broadcat, 0: P2P (Point To Point)
//in case of [7] == 1 => Broadcast then :
//[6] 1: Msg_Ack,  0: Req_Resp
//[5] 1: Message,  0: Acknowledge
//[5] 1: Request,  0: Response
//Broadcast has 7 bits => 128 Pids
//P2P       has 6 bits =>  64 Pids
//MAX Payload Size is 27 :
// + Retransmission needs 2 additionnal Bytes 
// + 1 for Size
// + 1 for CRC
//MAX Payload Size for P2P is 26 :
// Broadcast - 1
//as size is limited to 32 Bytes RF --> [bit7 = retrasnmit bit6..bit0 => size]

//data[0] == 0xDF => retransmit Header
//data[0] != 0xDF => data size Header

//---------------- Retransmit --------------
//Retransmit    : RTXPid TTL  PayloadX
#define rf_pid_0xDF_retransmit  0xDF

//---------------- Broadcast --------------
//Temperature   : Size Pid  SrcId  Payload2    CRC
//Alive         : Size Pid  SrcId  CRC
//Reset         : Size Pid  SrcId  CRC
//Light         : Size Pid  SrcId  Payload2 CRC
//Magnet        : Size Pid  SrcId  CRC
//BME280        : Size Pid  SrcId  Payload7 CRC
//---------------- P2P --------------------
//Protocol      : Size BRPid  Src Dest Payload CRC
//Ping          : Size Pid  SrcId DstId CRC
//Pong          : Size Pid  SrcId DstId CRC
//RGB           : Size Pid  SrcId TrgId  Payload3 CRC
//SwitchChan    : Size Pid  SrcId TrgId Payload
//ChanAck       : Size Pid  SrcId DstId
#define rfi_rt_ttl  0x01

#define rfi_size    0x00
#define rfi_pid     0x01
#define rfi_src     0x02
#define rfi_dst     0x03
#define rfi_retransmit_header_size   0x02
#define rfi_broadcast_header_size   0x03
#define rfi_broadcast_payload_offset   0x03
#define rfi_header_size   0x04
#define rfi_payload_offset 0x04

//---------------- Broadcast --------------
#define rf_pid_0xB5_temperature	0xB5
#define rf_pid_0xF5_alive		0xF5
#define rf_pid_0xC9_reset		0xC9
#define rf_pid_0xBB_light		0xBB
#define rf_pid_0xC5_magnet		0xC5
#define rf_pid_0xE2_bme280		0xE2

//---------------- P2P --------------
//---Messages_With_Acknowledge [6] = 1
//---Messages [5] = 1
#define rf_pid_rgb 		                0x19
#define rf_pid_ping                     0x04
#define rf_pid_0x67_test_switchChan     0x67
//---Acknowledge [5] = 0
#define rf_pid_0x41_test_chanAck        0x41
#define rf_pid_0x44_test_pong           0x44
// => Empty response is an Acknowledge => Check Size




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

