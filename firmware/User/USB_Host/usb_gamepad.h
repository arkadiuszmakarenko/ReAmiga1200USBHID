
#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include <stdint.h>
#include "usb_host_config.h"

 typedef struct _HID_gamepad_Info
 {
   uint8_t gamepad_data;
   uint8_t gamepad_extraBtn;
 }
 HID_gamepad_Info_TypeDef;

 HID_gamepad_Info_TypeDef *GetGamepadInfo(Interface *Itf);
 USBH_StatusTypeDef GamepadDecode(Interface *Itf);
#endif
