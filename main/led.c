#include "led.h"
#include "driver/gpio.h"
led_hw led_buff[MAX_LED]={0};
 
void led_on(uint8_t led);
void led_off(uint8_t led);
//10ms
void LED_IRQHandle(void)
{
	uint8_t i=0;
	for(i=0;i<MAX_LED;i++)
	{
		led_buff[i].counter+=10;
		if(led_buff[i].on_off==1)
		{
			if(led_buff[i].counter>led_buff[i].period)
			{
				led_buff[i].counter=0;
				led_on(i);
			}else if(led_buff[i].counter>led_buff[i].duty)
			{
				 led_off(i);
			}
		}
		else
		{   
				led_off(i);
				led_buff[i].on_off = 0;
				led_buff[i].period = 0;
				led_buff[i].duty = 0;
		}
		
	}
		
}
void register_led(uint8_t led ,uint16_t pin,uint8_t active_level)
{
	
		led_buff[led].pin = pin;
		led_buff[led].active_level = active_level;
}
void startLed(uint8_t led ,uint16_t period,uint16_t duty)
{
	led_buff[led].on_off = 1;
	led_buff[led].period = period;
	led_buff[led].duty = duty;
}

void stopLed(uint8_t led)
{
	led_buff[led].on_off = 0;
}
void led_on(uint8_t led)
{
 
	gpio_set_level(GPIO_ID_PIN(led_buff[led].pin), led_buff[led].active_level);  

}
void led_off(uint8_t led)
{

		if(led_buff[led].active_level==0)
		{
			 gpio_set_level(GPIO_ID_PIN(led_buff[led].pin), 1);  
 
		}
		else
		{
			 gpio_set_level(GPIO_ID_PIN(led_buff[led].pin), 0);  
 
		}
}

void led_task(void *pvParameters)
{
	while(1)
	{
		LED_IRQHandle();
		vTaskDelay(10/portTICK_RATE_MS);   
	}
}