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



#include "asm_5915.h"
#include "common.h"
#include "driver/i2c.h"
#include "driver/hw_timer.h"
#define I2C_EXAMPLE_MASTER_SCL_IO           2               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           14               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define AMS5915 		    
 
#define I2C_AMS5915_ADD    0x28
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
/**
 * @brief i2c master initialization
 */
unsigned char databuf[4]={0};
float press=0;
float ams5915_p;
float ams5915_t;
int temp_t=0;
static const char *TAG = "asm";
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
esp_err_t I2C_Readbuff( unsigned char I2C_Addr,unsigned char *buf)
{    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
    i2c_master_write_byte(cmd, I2C_Addr<<1|I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, buf, 4, LAST_NACK_VAL);
	i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
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
extern     char str_asm[80];
float I2C_AMS5915_Read(void)
{
	static short count=0;
     I2C_Readbuff( I2C_AMS5915_ADD,databuf);
	temp_t=(databuf[2]<<3)|(databuf[3]>>5);
	ams5915_t=(temp_t*200.0)/2048-50;
    // ESP_LOGI(TAG, "ams5915_t is %d\n",(int )(ams5915_t*100));
	press=((databuf[0]&0x3f)<<8)|databuf[1];
	count++;
	if(count>50)
	{count=0;}
	ams5915_p=((press-1638)/((14745-1638)/5));//*0.04+ams5915_p*0.96;
    // ESP_LOGI(TAG, "ams5915_p is %d\n",(int )(ams5915_p*100));
    sprintf(str_asm, "ams5915_p = %d\r\nams5915_t = %d \r\n", (int )(ams5915_p*100),(int )(ams5915_t*100));
   
	return ams5915_p;
	
}
void I2C_AMS5915_Read_Task(void *pvParameters)
    {
    printf("i2c_example_master_init:%d\n", i2c_example_master_init());
    while(1)
    {
        I2C_AMS5915_Read();
 
        vTaskDelay(1000/portTICK_RATE_MS);   
    }
}