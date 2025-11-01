#ifndef __UTILS_H
#define __UTILS_H

#include "debug.h"
#include "ch32v20x_gpio.h"
#include "ch32v20x_usbfs_host.h"


void blinkLed(int number,int delayms);
void InitStuff(void);
void DeInitStuff(void);

#endif /* __UTILS_H */
