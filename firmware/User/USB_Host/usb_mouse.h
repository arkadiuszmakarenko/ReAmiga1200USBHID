#ifndef __USB_MOUSE_H
#define __USB_MOUSE_H

#include <stdint.h>
#include "usb_host_config.h"
#include "utils.h"

typedef struct _HID_MOUSE_Info
{
  int16_t              x;
  int16_t              y;
  int8_t              buttons[3];
  int16_t             wheel;
}
HID_MOUSE_Info_TypeDef;

HID_MOUSE_Info_TypeDef *USBH_GetMouseInfo(Interface *Itf);
USBH_StatusTypeDef USBH_MouseDecode(Interface *Itf);


#endif
