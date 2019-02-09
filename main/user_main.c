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
#include "esp_smartconfig.h"

#include "led.h"
#include "multi_button.h"
#include "asm_5915.h"
#include "common.h"
#include "http_request.h"
uint8_t sta_mac[8] = {0};
static const char *TAG = "idle";
 uint8_t sta_mac_str[18] = {0};
 void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt);


 


void app_main()
{
    
    // cJSON * root =  cJSON_CreateObject();
    // cJSON * item =  cJSON_CreateObject();
    // cJSON * next =  cJSON_CreateObject();

    // cJSON_AddItemToObject(root, "rc", cJSON_CreateNumber(0));//根节点下添加
    // cJSON_AddItemToObject(root, "operation", cJSON_CreateString("CALL"));
    // cJSON_AddItemToObject(root, "service", cJSON_CreateString("telephone"));
    // cJSON_AddItemToObject(root, "text", cJSON_CreateString("打电话给张三"));
    // cJSON_AddItemToObject(root, "semantic", item);//root节点下添加semantic节点
    // cJSON_AddItemToObject(item, "slots", next);//semantic节点下添加item节点
    // cJSON_AddItemToObject(next, "name", cJSON_CreateString("张三"));//添加name节点

 
 
 
    esp_read_mac(sta_mac, ESP_MAC_WIFI_STA);
    hex_str(sta_mac, 6, sta_mac_str);
    xTaskCreate(led_task, "led_task", 1024, NULL, 2, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 2, NULL);
    xTaskCreate(I2C_AMS5915_Read_Task, "I2C_AMS5915_Read_Task", 2048, NULL, 2, NULL);
    xTaskCreate(&http_get_task, "http_get_task", 16384, NULL, 5, NULL);
  
    Init_time();
 while(1){
           ESP_LOGI(TAG,"free heap = %d\r\n", esp_get_free_heap_size());
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