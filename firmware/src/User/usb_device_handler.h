#ifndef USB_DEVICE_HANDLER_H
#define USB_DEVICE_HANDLER_H

#include <stdint.h>
#include "usb_host_config.h"

#define MAX_HUB_DEVICES 5

void usb_init(void);
void usb_process_devices(void);

#endif
