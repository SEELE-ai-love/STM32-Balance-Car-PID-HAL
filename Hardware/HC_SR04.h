#ifndef __HC_SR04_H
#define __HC_SR04_H

#include "main.h"

/* 引脚定义（可按实际接线修改）
 * 建议：TRIG -> PA8，ECHO -> PA11
 * 电源：VCC -> 5V，GND -> GND
 */
#define HC_SR04_TRIG_GPIO    GPIOA
#define HC_SR04_TRIG_PIN     GPIO_PIN_8
#define HC_SR04_ECHO_GPIO    GPIOA
#define HC_SR04_ECHO_PIN     GPIO_PIN_11

void HC_SR04_Init(void);              /* 初始化超声波模块引脚 */
float HC_SR04_GetDistance(void);      /* 获取距离，单位cm，超时返回-1 */

#endif
