/* OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "nvs.h"
#include "nvs_flash.h"


#include "led.h"
#include "multi_button.h"
#include "asm_5915.h"
#include "common.h"
#include "http_request.h"
#if CONFIG_SSL_USING_WOLFSSL
#include "lwip/apps/sntp.h"
#endif

static const char *TAG = "simple_ota_example";
 const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
 const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");


#if CONFIG_SSL_USING_WOLFSSL
static void get_time()
{
    struct timeval now;
    int sntp_retry_cnt = 0;
    int sntp_retry_time = 0;

    sntp_setoperatingmode(0);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    while (1) {
        for (int32_t i = 0; (i < (SNTP_RECV_TIMEOUT / 100)) && now.tv_sec < 1525952900; i++) {
            vTaskDelay(100 / portTICK_RATE_MS);
            gettimeofday(&now, NULL);
        }

        if (now.tv_sec < 1525952900) {
            sntp_retry_time = SNTP_RECV_TIMEOUT << sntp_retry_cnt;

            if (SNTP_RECV_TIMEOUT << (sntp_retry_cnt + 1) < SNTP_RETRY_TIMEOUT_MAX) {
                sntp_retry_cnt ++;
            }

            ESP_LOGI(TAG, "SNTP get time failed, retry after %d ms\n", sntp_retry_time);
            vTaskDelay(sntp_retry_time / portTICK_RATE_MS);
        } else {
            ESP_LOGI(TAG, "SNTP get time success\n");
            break;
        }
    }
}
#endif

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}




void simple_ota_task(void * pvParameter)
{
    ESP_LOGI(TAG, "Starting OTA example...");
    initialise_wifi();
     while(wifi_event_group==NULL){
        /* code */
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    #if CONFIG_SSL_USING_WOLFSSL
    /* CA date verification need system time */
    get_time();
    #endif
    
    /* Wait for the callback to set the CONNECTED_BIT in the
       event group.
    */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connect to Wifi ! Start to Connect to Server....");
    
    esp_http_client_config_t config = {
        .url = "https://192.168.0.107:8070/hello-world.bin",
        .cert_pem = (char *)server_cert_pem_start,
        .event_handler = _http_event_handler,
    };
    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
         startLed(LED_RED ,300,150);
         startLed(LED_BLUE ,300,150);
         vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware Upgrades Failed");
        startLed(LED_RED ,300,299);
        startLed(LED_BLUE ,300,299);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    xTaskCreate(button_task, "button_task", 2048, NULL, 2, NULL);
    xTaskCreate(I2C_AMS5915_Read_Task, "I2C_AMS5915_Read_Task", 2048, NULL, 2, NULL);
    xTaskCreate(&http_get_task, "http_get_task", 16384, NULL, 5, NULL);
    while(1)
	{
		 
		vTaskDelay(10000/portTICK_RATE_MS);  //send every 1 seconds
	}
}

