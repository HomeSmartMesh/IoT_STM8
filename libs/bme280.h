/** @file bme280.h
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


BYTE bme280_read_reg(BYTE address);

void bme280_read_registers(BYTE Start, BYTE Number,BYTE *data);

void bme280_print_registers(BYTE Start, BYTE Number);

void bme280_print_CalibData();

void bme280_check_id();

void bme280_force_OneMeasure(BYTE Press,BYTE Temp,BYTE Hum);

BYTE bme280_wait_measures();

void bme280_print_status();

void bme280_print_measures();

//void bme280_get_measures(BYTE *measures);

//get the measures from the sensor and format them for tx
//make sure the payload is a pre allocated 8 bytes buffer
void bme280_get_tx_payload_8B(BYTE *payload);

void bme280_rx_measures(BYTE src_NodeId,BYTE *rxPayload,BYTE rx_PayloadSize);//Rx 11 Bytes
