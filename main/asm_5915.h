
#ifndef	_ASM5915_H_
#define _ASM5915_H_

#include "stdint.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
 void I2C_AMS5915_Read_Task(void *pvParameters);
 char * generate_str(void);
 bool check_update(void);
#endif

