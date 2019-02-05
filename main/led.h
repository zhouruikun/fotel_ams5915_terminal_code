
#ifndef	_LED_H_
#define _LED_H_

#include "stdint.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
typedef enum
{
     LED_BLUE,
	 MAX_LED
}led;
typedef struct 
{
	uint16_t pin;
	uint8_t active_level;
	uint16_t period;
	uint16_t duty;
	uint16_t counter;
	uint8_t on_off;
}led_hw;
void register_led(uint8_t led ,uint16_t pin,uint8_t active_level);
void startLed(uint8_t led ,uint16_t period,uint16_t duty);
void stopLed(uint8_t led);
void LED_IRQHandle(void);
void led_task(void *pvParameters);
#endif

