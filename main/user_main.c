/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>

#include "esp_system.h"

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

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
#include "driver/gpio.h"
#include "esp_smartconfig.h"
#include "driver/uart.h"

#include "led.h"
#include "multi_button.h"
#include "asm_5915.h"
#include "common.h"
#include "http_request.h"
uint8_t sta_mac[8] = {0};
static const char *TAG = "idle";
 uint8_t sta_mac_str[18] = {0};
 void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt);
 void simple_ota_task(void * pvParameter);

#define GPIO_OUTPUT_IO_0    12
#define GPIO_OUTPUT_IO_1    14
#define GPIO_OUTPUT_IO_KEY    13
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
 

void app_main()
{
    esp_read_mac(sta_mac, ESP_MAC_WIFI_STA);

        // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    hex_str(sta_mac, 6, sta_mac_str);

    gpio_config_t io_conf;
      //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
     //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
      //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
     //disable pull-down mode
    io_conf.pull_down_en = 0;
     //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    initialise_wifi();
    register_led(LED_BLUE ,GPIO_OUTPUT_IO_1,0);//led指示灯
    register_led(LED_RED ,GPIO_OUTPUT_IO_0,0);//led指示灯
    xTaskCreate(led_task, "led_task", 1024, NULL, 2, NULL);
        xTaskCreate(button_task, "button_task", 2048, NULL, 2, NULL);
    xTaskCreate(I2C_AMS5915_Read_Task, "I2C_AMS5915_Read_Task", 2048, NULL, 2, NULL);
    xTaskCreate(&http_get_task, "http_get_task", 16384, NULL, 5, NULL);
    // xTaskCreate(simple_ota_task, "ota_example_task", 8192, NULL, 5, NULL);//开机进行升级  
    Init_time();
    while(1){
              //  ESP_LOGI(TAG,"free heap = %d\r\n", esp_get_free_heap_size());
              vTaskDelay(1000/portTICK_RATE_MS);   
        /* code */
    }
}

 
void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt)
{
   unsigned char hbit,lbit;
   unsigned int i;
   for(i=0;i<len;i++)
    {
      hbit = (*(inchar+i)&0xf0)>>4;
      lbit = *(inchar+i)&0x0f;
      if (hbit>9)
          outtxt[2*i]='A'+hbit-10;
        else
          outtxt[2*i]='0'+hbit;
      if (lbit>9)
          outtxt[2*i+1]='A'+lbit-10;
        else
          outtxt[2*i+1]='0'+lbit;
    }
    outtxt[2*i] = 0;
}