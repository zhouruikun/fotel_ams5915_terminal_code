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

#include "common.h"
#include <math.h>
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/hw_timer.h"
#define I2C_EXAMPLE_MASTER_SCL_IO 4         /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO 5         /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM I2C_NUM_0    /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */

#define AMS5915
#define DEBUG 0
#define I2C_AMS5915_ADD 0x28
#define LAST_NACK_VAL 0x2 /*!< I2C last_nack value */
#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/

#define MAX_POINT 20
/**
 * @brief i2c master initialization
 */
unsigned char databuf[4] = {0};
double press = 0;
double ams5915_p[MAX_POINT];
double ams5915_t[MAX_POINT];
int speed_one_point;
int speed_offset;
int temperature_one_point;
int count_point = 0;
cJSON *item;
cJSON *root;
cJSON *data_array_press;
cJSON *data_array_temp;
double temperature = 0;
static const char *TAG = "asm";
static double last_temp = 0;
static double last_ams = 0;
static esp_err_t i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}

/************************************************************************************
**º¯ Êý Ãû: 
**¹¦    ÄÜ: Í¨¹ýÖ¸¶¨I2C½Ó¿Ú¶ÁÈ¡¶à¸ö×Ö½ÚÊý¾Ý
**Èë¿Ú²ÎÊý: DK_I2CxÎª1±íÊ¾Ê¹ÓÃIIC×ÜÏß1£¬Îª2±íÊ¾ÊÇ×ÜÏß2£¬´ÓÆ÷¼þµØÖ·,addr Ô¤¶ÁÈ¡×Ö½Ú´æ´¢Î»ÖÃ£¬
			*buf ¶ÁÈ¡Êý¾ÝµÄ´æ´¢Î»ÖÃ£¬num¶ÁÈ¡×Ö½ÚÊý,addrLenDworÎª1±íÊ¾´æ´¢Î»ÖÃµØÖ·¿í¶È´óÓÚ1¸ö×Ö½Ú£¬Îª0Ôò±íÊ¾1¸ö×Ö½Ú
**         
**³ö¿Ú²ÎÊý: ³É¹¦·µ»Ø0
************************************************************************************/
esp_err_t I2C_Readbuff(unsigned char I2C_Addr, unsigned char *buf)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_Addr << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, buf, 4, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "i2c_master_cmd_begin error =%d", ret);

        return ret;
    }
    return ESP_OK;
}
/************************************************************************************
**º¯ Êý Ãû: unsigned char I2C_EEPROM_Read(unsigned int ReadAddr, unsigned char* pBuffer, unsigned int NumByteToRead)
**¹¦    ÄÜ: 
**Èë¿Ú²ÎÊý: 
**         
**³ö¿Ú²ÎÊý: ³É¹¦·µ»Ø0
************************************************************************************/
extern char str_asm[80];
double I2C_AMS5915_Read(void)
{
    static short count_filter = 0;
    static double temperature_last = 0;
    static double press_last = 0;
    I2C_Readbuff(I2C_AMS5915_ADD, databuf);
    temperature = (databuf[2] << 3) | (databuf[3] >> 5);
    temperature = (temperature * 200.0) / 2048 - 50;
    temperature = temperature * 0.1 + temperature_last * 0.9;
    temperature_last = temperature;
    press = ((databuf[0] & 0x3f) << 8) | databuf[1];
    press = ((press - 1638) / ((14745 - 1638) / 5)); //*0.04+ams5915_p*0.96;
    press = press * 0.1 + press_last * 0.9;
    press_last = press;
    count_filter++;
    if (count_filter > 10)
    {
        if (DEBUG) {
            
            ESP_LOGI(TAG, "speed_one_point:%d press:%d speed_offset:%d\n", speed_one_point,(int)(press*100),speed_offset);
        }
        
        // 
        //自适应算法 结合心跳包上传
        if (press > 0)
        {
            /* code */
            speed_one_point = (0.83 * sqrt(2 * (press * 100) / 1.293) - 0.2) * 100; //press*100;//计算风速(0.83*Math.sqrt(2*(stress)/1.293)-0.2).toFixed(2);
        }
        else
        {
            speed_one_point = -(0.83 * sqrt(2 * (-press * 100) / 1.293) - 0.2) * 100; //press*100;//计算风速(0.83*Math.sqrt(2*(stress)/1.293)-0.2).toFixed(2);
        }
        speed_one_point -= speed_offset;
        temperature_one_point = temperature * 100;
        count_filter = 0;
        ams5915_p[count_point] = press * 100;
        ams5915_t[count_point] = temperature * 100;
        count_point++;
        if (count_point >= MAX_POINT)
            count_point = 0;
    }
    return ams5915_p[count_point];
}

#define BUF_SIZE 256
void I2C_AMS5915_Read_Task(void *pvParameters)
{
    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL);
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    uint8_t *str = (uint8_t *)malloc(BUF_SIZE);
    //A5 5A 07 82 00 00 00 01 00 02
    data[0] = 0xa5;
    data[1] = 0x5a;
    data[2] = 0x0b;
    data[3] = 0x82;
    data[4] = 0;
    data[5] = 0;
    data[10] = 0;
    data[11] = 0x00;
    ESP_LOGI(TAG, "i2c_example_master_init:%d\n", i2c_example_master_init());
    short cnt = 0;
    while (1)
    {
        I2C_AMS5915_Read();
        short temp_t_i = (short)(temperature * 100);
        short press_i = (short)(press * 10000);
        data[6] = ((temp_t_i & 0xff00) >> 8);
        data[7] = ((temp_t_i & 0x00ff));
        data[8] = ((press_i & 0xff00) >> 8);
        data[9] = ((press_i & 0x00ff));
        cnt++;
        if (cnt > 5)
        {
            cnt = 0;
            data[11] ^= 0x01;
        }
        if (abs(speed_one_point) > 5)
        {
            /* code */
            data[12] = ((speed_one_point & 0xff00) >> 8);
            data[13] = ((speed_one_point & 0x00ff));
        }
        else
        {
            data[12] = 0;
            data[13] = 0;
        }

        hex_str(data, 14, str);
        if (DEBUG ==0) {
           uart_write_bytes(UART_NUM_0, (const char *)data, 14);
        }
        
         
        // uart_write_bytes(UART_NUM_0, (const char *) str, 28);
        // int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

// void I2C_AMS5915_Read_Task(void *pvParameters)
// {

//    ESP_LOGI(TAG,"i2c_example_master_init:%d\n", i2c_example_master_init());
//     while(1)
//     {
//         I2C_AMS5915_Read();
//         vTaskDelay(100/portTICK_RATE_MS);
//     }
// }
#include <time.h>
#include <sys/time.h>
char *generate_str(void)
{
    time_t now_time = 0;
    char *str_request;
    time(&now_time);
    root = cJSON_CreateObject();
    item = cJSON_CreateObject();
    last_temp = ams5915_t[count_point - 1];
    last_ams = ams5915_p[count_point - 1];

    data_array_press = cJSON_CreateDoubleArray(ams5915_p, count_point);
    data_array_temp = cJSON_CreateDoubleArray(ams5915_t, count_point);

    cJSON_AddItemToObject(root, "nodeMac", cJSON_CreateString((char *)&sta_mac_str)); //根节点下添加
    cJSON_AddItemToObject(root, "updateTime", cJSON_CreateNumber(now_time));          //根节点下添加
    cJSON_AddItemToObject(root, "nodeType", cJSON_CreateString("AMS"));               //根节点下添加
    cJSON_AddItemToObject(root, "nodeDataCounts", cJSON_CreateNumber(count_point));
    cJSON_AddItemToObject(root, "data", item); //root节点下添加semantic节点

    cJSON_AddItemToObject(item, "ams5915_p", data_array_press);
    cJSON_AddItemToObject(item, "ams5915_t", data_array_temp);

    str_request = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Delete(item);
    cJSON_Delete(data_array_press);
    cJSON_Delete(data_array_temp);
    count_point = 0;
    return str_request;
}

char *generate_strforpoint(void)
{
    time_t now_time = 0;
    char *str_request;
    time(&now_time);
    root = cJSON_CreateObject();
    item = cJSON_CreateObject();
    last_temp = ams5915_t[count_point - 1];
    last_ams = ams5915_p[count_point - 1];

    data_array_press = cJSON_CreateDoubleArray(ams5915_p, count_point);
    data_array_temp = cJSON_CreateDoubleArray(ams5915_t, count_point);

    cJSON_AddItemToObject(root, "nodeMac", cJSON_CreateString((char *)&sta_mac_str)); //根节点下添加
    cJSON_AddItemToObject(root, "updateTime", cJSON_CreateNumber(now_time));          //根节点下添加
    cJSON_AddItemToObject(root, "nodeType", cJSON_CreateString("AMS"));               //根节点下添加
    cJSON_AddItemToObject(root, "nodeDataCounts", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(root, "data", item); //root节点下添加semantic节点
    cJSON_AddItemToObject(item, "ams5915_p", cJSON_CreateNumber(speed_one_point));
    cJSON_AddItemToObject(item, "ams5915_t", cJSON_CreateNumber(temperature_one_point));

    str_request = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Delete(item);
    cJSON_Delete(data_array_press);
    cJSON_Delete(data_array_temp);
    count_point = 0;
    return str_request;
}
#define GAP_TEMP 20
#define GAP_AMS 2
#define HEART_PACK 2
bool check_update(void)
{
    //定时判断是否需要上传数据 标准为本次数据与上次上传数据差值是否大于范围 外加定时2分钟一个心跳包
    static time_t time_start = 0;
    static uint8_t init_flag = 1;
    time_t now_time_end = 0;
    char *str_request;
    time(&now_time_end);
    if (abs(last_temp - temperature * 100) > GAP_TEMP || abs(last_ams - press * 100) > GAP_AMS || (now_time_end - time_start) > 60 * HEART_PACK)
    {
        if ((now_time_end - time_start) > 60 * HEART_PACK)
        { //扣除零点
            if (init_flag == 0)
            {
                speed_offset += speed_one_point;
            }
            else
            {
                init_flag = 0;
            }

            /* code */
        }

        time(&time_start);
        return true;
    }
    else
    {
        count_point = 0;
        return false;
    }
}