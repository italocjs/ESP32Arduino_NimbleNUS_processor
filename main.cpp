/**
 * @file main.cpp
 * Thanks to the author of NuPacket library Ángel Fernández Pineda
 * Example ported and expanded by Italo Soares.
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "NuPacket.hpp"

uint16_t current_mtu = 0;    // 0 = not connected to anyone, should be between 23 and 517

void start_and_print_info()
{
	// Initialize serial monitor
	Serial.begin(115200);

	/* Print chip information */
	esp_chip_info_t chip_info;
	uint32_t flash_size;
	esp_chip_info(&chip_info);
	ESP_LOGI("BOOT", "This is %s chip with %d CPU core(s), %s%s%s%s, ", CONFIG_IDF_TARGET, chip_info.cores,
	         (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "", (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
	         (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
	         (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

	unsigned major_rev = chip_info.revision / 100;
	unsigned minor_rev = chip_info.revision % 100;
	ESP_LOGI("BOOT", "silicon revision v%d.%d, ", major_rev, minor_rev);
	if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
	{
		ESP_LOGE("BOOT", "Get flash size failed");
		return;
	}

	ESP_LOGI("BOOT", "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
	         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	ESP_LOGI("BOOT", "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}

QueueHandle_t rxQueue;
QueueHandle_t txQueue;
#define BUFFER_SIZE 517    // Adjust based on your requirements

void debug_connection_status()
{
	ESP_LOGI("RX TASK", "Debugging connection status...");

	// Check if NimBLEDevice initialized successfully
	if (!NimBLEDevice::getInitialized())
	{
		ESP_LOGW("RX TASK", "Warning: NimBLEDevice not initialized properly.");
		return;    // Exit the function if not initialized to avoid further errors
	}
	else
	{
		ESP_LOGI("RX TASK", "NimBLEDevice initialized successfully.");
	}

	// Device Address
	NimBLEAddress deviceAddress = NimBLEDevice::getAddress();
	ESP_LOGI("RX TASK", "Device Address: %s", deviceAddress.toString().c_str());

// Power Level
#ifdef ESP_PLATFORM
	int power = NimBLEDevice::getPower(ESP_BLE_PWR_TYPE_DEFAULT);
	ESP_LOGI("RX TASK", "Power Level: %d", power);
#else
	ESP_LOGI("RX TASK", "Power Level information is not available on this platform.");
#endif

	// MTU Size
	uint16_t mtu = NimBLEDevice::getMTU();
	ESP_LOGI("RX TASK", "MTU Size: %d", mtu);

	// Whitelist Count
	size_t whitelistCount = NimBLEDevice::getWhiteListCount();
	ESP_LOGI("RX TASK", "Whitelist Count: %zu", whitelistCount);

	// Whitelist Addresses
	for (size_t i = 0; i < whitelistCount; ++i)
	{
		NimBLEAddress whitelistAddress = NimBLEDevice::getWhiteListAddress(i);
		ESP_LOGI("RX TASK", "Whitelist Address %zu: %s", i + 1, whitelistAddress.toString().c_str());
	}
}

typedef struct
{
	size_t size;
	uint8_t* data;
} TxItem;

#include "esp_log.h"
#include <ctype.h>

void printDataAsAsciiAndHex(const char* tag, const uint8_t* data, size_t size) {
    const size_t bytesPerLine = 16;
    char lineBuffer[bytesPerLine * 3 + bytesPerLine + 3 + 1]; // Hex + ASCII + spaces and '|' + null terminator
    size_t dataIndex = 0;

    while (dataIndex < size) {
        size_t lineBufferIndex = 0;
        for (size_t i = 0; i < bytesPerLine; ++i) {
            if (dataIndex + i < size) {
                snprintf(&lineBuffer[lineBufferIndex], 4, "%02X ", data[dataIndex + i]); // Hex part
                lineBufferIndex += 3; // 2 hex digits and a space
            } else {
                // Pad the rest of the hex part if we're at the end
                snprintf(&lineBuffer[lineBufferIndex], 4, "   ");
                lineBufferIndex += 3;
            }
        }

        lineBuffer[lineBufferIndex++] = '|'; // Separator between hex and ASCII

        for (size_t i = 0; i < bytesPerLine; ++i) {
            if (dataIndex + i < size) {
                char c = data[dataIndex + i];
                lineBuffer[lineBufferIndex++] = isprint((unsigned char)c) ? c : '.';
            } else {
                lineBuffer[lineBufferIndex++] = ' '; // Pad the rest if we're at the end
            }
        }

        lineBuffer[lineBufferIndex++] = '|'; // Closing separator
        lineBuffer[lineBufferIndex] = '\0'; // Ensure null-terminated string

        ESP_LOGI(tag, "%s", lineBuffer);
        dataIndex += bytesPerLine;
    }
}

void NimBLE_rxTask(void* pvParameters)
{
	ESP_LOGI("NimBLE RX", "NimBLE_rxTask starting, current free heap: %d, minimum free heap: %d", esp_get_free_heap_size(),
	         esp_get_minimum_free_heap_size());

	// Initialize BLE stack and Nordic UART service
	NimBLEDevice::init("NuPacket demo");
	NimBLEDevice::setPower(ESP_PWR_LVL_P9);
	NuPacket.start();
	debug_connection_status();

	ESP_LOGI("NimBLE RX", "NimBLE_rxTask started, BLE enabled, current free heap: %d, minimum free heap: %d", esp_get_free_heap_size(),
	         esp_get_minimum_free_heap_size());

	while (1)
	{
        ESP_LOGI("NimBLE RX", "Waiting for connection...");
		if (NuPacket.connect())
		{
            ESP_LOGI("NimBLE RX", "Connected!");
			size_t size;

			const uint8_t* data = NuPacket.read(size);
			while (data)
			{

                #define DEBUG_BLE_RX_INFO

                #ifdef DEBUG_BLE_RX_INFO 
                ESP_LOGI("NimBLE RX", "MTU: %d Data packet size %d bytes",NuPacket.getMTU(), size); 
                printDataAsAsciiAndHex("NimBLE RX", data, size);
                ESP_LOGI("NimBLE RX", "--end of packet--");
                #endif

				// Prepare data for TX task
				TxItem* item = (TxItem*)malloc(sizeof(TxItem));
				if (item == nullptr)
				{
                    ESP_LOGE("NimBLE RX", "Failed to allocate memory for TX item");
					continue;    // Skip this packet
				}

				item->size = size;
				item->data = (uint8_t*)malloc(size);
				if (item->data == nullptr)
				{
                    ESP_LOGE("NimBLE RX", "Failed to allocate memory for TX data");
					free(item);    // Clean up previously allocated memory
					continue;      // Skip this packet
				}
				memcpy(item->data, data, size);

				// Send to TX task
				if (xQueueSend(txQueue, &item, portMAX_DELAY) != pdPASS)
				{
                    ESP_LOGE("NimBLE RX", "Failed to send item to TX task");
					free(item->data);    // Clean up
					free(item);          // Clean up
				}

				// Receive next packet
				data = NuPacket.read(size);
			}
            ESP_LOGI("NimBLE RX", "Disconnected");
		}
	}
}

void NimBLE_txTask(void* pvParameters)
{
    ESP_LOGI("NimBLE TX", "NimBLE_rxTask starting, current free heap: %d, minimum free heap: %d", esp_get_free_heap_size(),
	         esp_get_minimum_free_heap_size());
             
	while (1)
	{
		TxItem* receivedItem = nullptr;
		// Wait for data to be received in the txQueue
		if (xQueueReceive(txQueue, &receivedItem, portMAX_DELAY) == pdPASS)
		{
			size_t dataSize = receivedItem->size;
			uint8_t* data = receivedItem->data;

			// Assuming MTU is known and considering null terminator
			const size_t packageSize = NuPacket.getMTU() - 1;    // -1 to ensure space for null terminator

			size_t bytesSent = 0;
			while (bytesSent < dataSize)
			{
				size_t currentPackageSize = ((dataSize - bytesSent) < packageSize) ? (dataSize - bytesSent) : packageSize;

				// Temporarily allocate buffer for current package + null terminator
				char* packageData = (char*)malloc(currentPackageSize + 1);    // +1 for null terminator
				if (packageData == nullptr)
				{
					ESP_LOGW("NimBLE TX", "Failed to allocate memory for package");
					break;    // Exit the loop on allocation failure
				}

				// Prepare the package with a null terminator
				memcpy(packageData, data + bytesSent, currentPackageSize);
				packageData[currentPackageSize] = '\0';    // Null-terminate the string

				// Send the package data
				if (NuPacket.send(packageData, false) == 0)
				{
					ESP_LOGW("NimBLE TX", "Failed to send package data or no peer connected");
				}

				free(packageData);    // Free the temporary buffer

				bytesSent += currentPackageSize;
			}

			// Free the data buffer and the TxItem structure after processing
			free(data);            // Free the data buffer inside the TxItem
			free(receivedItem);    // Free the TxItem structure itself
		}
	}
}

void setup()
{
	start_and_print_info();

	// create the queues used
	// rxQueue = xQueueCreate(10, sizeof(uint8_t));
	txQueue = xQueueCreate(10, sizeof(TxItem*));

	// create the tasks
	xTaskCreate(NimBLE_rxTask, "NimBLE_rxTask", 2048, NULL, 5, NULL);
	xTaskCreate(NimBLE_txTask, "NimBLE_txTask", 2048, NULL, 5, NULL);
}

void loop()
{
	Serial.println("loop is alive");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}
