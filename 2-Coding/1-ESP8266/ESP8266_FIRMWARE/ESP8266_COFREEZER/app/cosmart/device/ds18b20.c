/*
 * ds18b20.c
 *
 *  Created on: 2017-02-17 13:18:04
 *      Author: Cocoonshu
 */

#include "cosmart/device/ds18b20.h"
#include "cosmart/log.h"
#include "mem.h"
#include "user_interface.h"
#include "driver/onewire.h"

LOCAL void ICACHE_FLASH_ATTR searchDevicesOnBus();

typedef struct DS18B20 {
	uint8* rom; // 8bit CRC + 48bit serial + 8bit family code
	uint32 temperature; // temperature = C.degree * 10000
} DS18B20;

LOCAL uint32   mBusPort     = 0;
LOCAL DS18B20* mDeviceGroup = NULL;
LOCAL uint16   mDeviceCount = 0;


void ICACHE_FLASH_ATTR DS18B20_initialize(const uint32 busPort) {
	Log_printfln("");
    Log_printfln("[DS18B20] Start OneWire manager in master mode");


	mBusPort = busPort;
	searchDevicesOnBus();
}

LOCAL void ICACHE_FLASH_ATTR searchDevicesOnBus() {
	onewire_init(mBusPort);
	onewire_reset_search(mBusPort);
	system_soft_wdt_feed();

	mDeviceCount = 0;
	if (mDeviceGroup != NULL) {
		os_free(mDeviceGroup);
		mDeviceGroup = NULL;
	}

	BOOL result = FALSE;
	do {
		uint8* rom = (uint8 *)os_zalloc(sizeof(uint8) * 8);
		BOOL   result  = onewire_search(mBusPort, rom);
		system_soft_wdt_feed();
		if (result) {
			mDeviceCount++;
			DS18B20* buffer = (DS18B20 *)os_zalloc(sizeof(DS18B20) * mDeviceCount);
			if (mDeviceGroup == NULL) {
				buffer[0].rom = rom;
				mDeviceGroup = buffer;
			} else {
				DS18B20* deletingBuffer = mDeviceGroup;

				int i = 0;
				for (i = 0; i < mDeviceCount - 1; i++) {
					buffer[i] = mDeviceGroup[i];
				}
				buffer[mDeviceCount - 1].rom = rom;
				deletingBuffer = mDeviceGroup;
				mDeviceGroup = buffer;
				os_free(deletingBuffer);
				deletingBuffer = NULL;
			}

			Log_printfln("[DS18B20] found device %d on bus %d", rom, mBusPort);
		}
	} while (result);
	onewire_reset_search(mBusPort);
}

uint16 ICACHE_FLASH_ATTR DS18B20_getDeviceCount() {
	return mDeviceCount;
}

float ICACHE_FLASH_ATTR DS18B20_getDeviceTemperature(const uint16 index) {
	if (index < 0 || index >= mDeviceCount) {
		return 0.0f;
	}

	system_soft_wdt_feed();

	DS18B20 device = mDeviceGroup[index];
	onewire_init(mBusPort);

	uint8 crc = onewire_crc8(device.rom, 7);
	if (crc == (device.rom[7])) {
		if (device.rom[0] == DS18S20_FAMILY || device.rom[0] == DS18B20_FAMILY) {
			onewire_reset(mBusPort);
			onewire_select(mBusPort, device.rom);
			onewire_write(mBusPort, CONVERT_TEMPERATURE, 1);

			uint8 present = onewire_reset(mBusPort);
			onewire_select(mBusPort, device.rom);
			onewire_write(mBusPort, READ_SCRATCHPAD, 1);

			uint8 buffer[9];
			onewire_read_bytes(mBusPort, buffer, 9);
			crc = onewire_crc8(buffer, 8);
			if (crc == buffer[8]) {
				uint32 temperature = buffer[0] + buffer[1] * 256;
				if (temperature > 32767) {
					temperature -= 65536;
				}
				if (device.rom[0] == DS18B20_FAMILY) {
					temperature *= 625;
				} else {
					temperature *= 5000;
				}
				device.temperature = temperature;

				Log_println("[DS18B20_getDeviceTemperature] Temperature = %f", (temperature / 10000.0f));
				return temperature / 10000.0f;
			} else {
				Log_println("[DS18B20_getDeviceTemperature] CRC failed! Reading cancel");
			}

		} else {
			Log_println("[DS18B20_getDeviceTemperature] This device is not DS18B20");
		}
	} else {
		Log_println("[DS18B20_getDeviceTemperature] CRC failed!");
	}

	system_soft_wdt_feed();

	return 0.0f;
}
