#ifndef __COMMON_H__
#define __COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "smart_config.h"
#define GPIO_INPUT_IO    13
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO))
uint8_t read_button1_GPIO() ;
#ifdef __cplusplus
}
#endif
#endif