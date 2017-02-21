/*
 * ds18b20.h
 *
 *  Created on: 2017-02-17 13:18:04
 *      Author: Cocoonshu
 */

#ifndef APP_INCLUDE_COSMART_DEVICE_DS18B20_H_
#define APP_INCLUDE_COSMART_DEVICE_DS18B20_H_

#include "c_types.h"

#define DS18S20_FAMILY      0x10
#define DS18B20_FAMILY      0x28
#define CONVERT_TEMPERATURE 0x44
#define READ_SCRATCHPAD     0xBE
#define WRITE_SCRATCHPAD    0x4E

void   DS18B20_initialize(const uint32 busPort);
uint16 DS18B20_getDeviceCount();
float  DS18B20_getDeviceTemperature(const uint16 index);

#endif /* APP_INCLUDE_COSMART_DEVICE_DS18B20_H_ */
