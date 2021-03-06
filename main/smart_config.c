

#include <string.h>
#include <stdlib.h>
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
#include "smart_config.h"
#include "spi_flash.h"


#include <time.h>
#include <sys/time.h>
#include "esp_attr.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"



#define ADDR_MYCONF 250 //the 250 sector
#define ADDR_SC 252 //the 250 sector

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */


const int CONNECTED_BIT = BIT0;
const int CONNECTING_BIT = BIT2;
const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "smart config";
wifi_config_t my_config;
void smartconfig_task(void *pvParameters);
void smartconfig_example_task(void * parm);
 static void obtain_time(void);
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        startLed(LED_BLUE ,2000,300);
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, CONNECTING_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTING_BIT);
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            startLed(LED_BLUE ,500,250);
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            startLed(LED_BLUE ,1000,500);
            ESP_LOGI(TAG, "SC_STATUS_LINK");
            wifi_config_t *wifi_config = pdata;
            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );


            spi_flash_erase_sector(ADDR_MYCONF);
			spi_flash_write(ADDR_MYCONF*4096,(uint32_t *)wifi_config,sizeof(wifi_config_t));

            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}

void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

void smartconfig_task(void *pvParameters)
{
	uint32_t sc_count=0;
     EventBits_t uxBits;
	if(pvParameters == NULL)
	{
		sc_count=0;
		spi_flash_read(ADDR_SC*4096,(uint32_t *)&sc_count,sizeof(sc_count));
		ESP_LOGI(TAG,"smartconfig_onoff sc_count=%u\r\n",sc_count);	
		if(sc_count>3)
		{
			ESP_LOGI(TAG,"smartconfig_start sc_count=%u\r\n",sc_count);	
			sc_count=0;
			spi_flash_erase_sector(ADDR_SC);
			spi_flash_write(ADDR_SC*4096,(uint32_t *)&sc_count,sizeof(sc_count));
            ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS) );
            ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
		}
		else
		{
			ESP_LOGI(TAG,"connecting......\r\n");	
			
			sc_count=sc_count+1;
			spi_flash_erase_sector(ADDR_SC);
			spi_flash_write(ADDR_SC*4096,(uint32_t *)&sc_count,sizeof(sc_count));

			spi_flash_read(ADDR_MYCONF*4096,(uint32_t *)&my_config,sizeof( wifi_config_t));
			sc_callback(SC_STATUS_LINK,(void *)&my_config);
			vTaskDelay(3000 / portTICK_RATE_MS);  //send every 1 seconds
            
			sc_count=0;
			spi_flash_erase_sector(ADDR_SC);
			spi_flash_write(ADDR_SC*4096,(uint32_t *)&sc_count,sizeof(sc_count));
		}
	}
	else
	{	
		ESP_LOGI(TAG,"smartconfig_start_button\r\n");	
        ESP_ERROR_CHECK( esp_smartconfig_stop() );
        ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS) );
        ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
	}
    
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & CONNECTED_BIT) {
            // obtain_time();
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}
