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
#include "cJSON.h"
#include "led.h"
#include "multi_button.h"
#include "asm_5915.h"
#include "common.h"
#include "http_request.h"

 
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

    // printf("%s\n", cJSON_Print(root));

    
    xTaskCreate(led_task, "led_task", 512, NULL, 2, NULL);
    xTaskCreate(button_task, "button_task", 1024, NULL, 2, NULL);
    xTaskCreate(I2C_AMS5915_Read_Task, "I2C_AMS5915_Read_Task", 1024, NULL, 2, NULL);
    xTaskCreate(&http_get_task, "http_get_task", 16384, NULL, 5, NULL);
    
    initialise_wifi();
}


 