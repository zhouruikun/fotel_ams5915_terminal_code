#ifndef __COMMON_H__
#define __COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "cJSON.h"
#include "asm_5915.h"
#include "smart_config.h"
#include "sntp_time.h"
#include "led.h"
#define GPIO_INPUT_IO    13
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO))
uint8_t read_button1_GPIO() ;
extern  uint8_t sta_mac_str[18] ;
void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt);
extern cJSON * item ;
extern cJSON * root ;

#ifdef __cplusplus
}
#endif
#endif