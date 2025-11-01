#ifndef __MOUSE_H
#define __MOUSE_H

#include "stdint.h"
#include "usb_mouse.h"
#include "utils.h"




#define MOUSEX	            0
#define MOUSEY	            1
#define Q_RATELIMIT         500
#define Q_BUFFERLIMIT       300
#define DPI_DIVIDER         2
#define CODE_MMB_UP         0b1110
#define CODE_MMB_DOWN       0b1101
#define CODE_WHEEL_UP       0b1011
#define CODE_WHEEL_DOWN     0b0111
#define CODE_WHEEL_LEFT     0b1010
#define CODE_WHEEL_RIGHT    0b0101
#define CODE_4TH_UP         0b1001
#define CODE_4TH_DOWN       0b1100
#define CODE_5TH_UP         0b0110
#define CODE_5TH_DOWN       0b0011


void InitMouse();
void ProcessMouse(HID_MOUSE_Info_TypeDef *mousemap);
void ProcessX_IRQ();
void ProcessY_IRQ();
void ProcessScrollIRQ();

#endif
