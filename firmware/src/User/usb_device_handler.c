#include "usb_device_handler.h"
#include <usb_gamepad.h>
#include <usb_mouse.h>
#include <usb_keyboard.h>
#include "mouse.h"
#include "keyboard.h"
#include "gamepad.h"
#include "utils.h"

static void process_hid_interface (Interface *interface) {
    if (interface == NULL) {
        return;
    }

    switch (interface->HIDRptDesc.type) {
    case REPORT_TYPE_MOUSE: {
        HID_MOUSE_Info_TypeDef *mousemap = USBH_GetMouseInfo (interface);
        ProcessMouse (mousemap);
        break;
    }
    case REPORT_TYPE_JOYSTICK: {
        HID_gamepad_Info_TypeDef *gamepad = GetGamepadInfo (interface);
        ProcessGamepad (gamepad);
        break;
    }
    case REPORT_TYPE_KEYBOARD: {
        HID_KEYBD_Info_TypeDef *kbd = USBH_HID_GetKeybdInfo (interface);
        amikb_process (kbd);
        break;
    }
    default:
        break;
    }
}

static void process_device_interfaces (uint8_t device_index) {
    for (int itf = 0; itf < DEF_INTERFACE_NUM_MAX; itf++) {
        process_hid_interface (&HostCtl[device_index].Interface[itf]);
    }
}

void usb_init (void) {
    DUG_PRINTF ("USBFS Host Init\r\n");
    USBFS_RCC_Init();
    DUG_PRINTF ("USBFS RCC Init OK\r\n");
    USBFS_Host_Init (ENABLE);
    DUG_PRINTF ("USBFS Host Init OK\r\n");
    memset (&RootHubDev.bStatus, 0, sizeof (ROOT_HUB_DEVICE));
    memset (&HostCtl[DEF_USBFS_PORT_INDEX * DEF_ONE_USB_SUP_DEV_TOTAL].InterfaceNum, 0,
            DEF_ONE_USB_SUP_DEV_TOTAL * sizeof (HOST_CTL));
    DUG_PRINTF ("USB structures initialized\r\n");
}

static uint8_t last_device_type = 0xFF;
static uint8_t last_status = 0xFF;
static uint8_t last_speed = 0xFF;
static uint8_t last_address = 0xFF;

void usb_process_devices (void) {
    USBH_MainDeal();

    // Detect any state changes
    if (RootHubDev.bStatus != last_status || RootHubDev.bType != last_device_type ||
        RootHubDev.bSpeed != last_speed || RootHubDev.bAddress != last_address) {

        if (RootHubDev.bStatus != last_status) {
            last_status = RootHubDev.bStatus;
        }

        if (RootHubDev.bType != last_device_type) {
            if (RootHubDev.bType == USB_DEV_CLASS_HID) {

            } else if (RootHubDev.bType == USB_DEV_CLASS_HUB) {

            } else if (RootHubDev.bType == 0) {

            } else {
            }
            last_device_type = RootHubDev.bType;
        }

        if (RootHubDev.bSpeed != last_speed) {
            last_speed = RootHubDev.bSpeed;
        }

        if (RootHubDev.bAddress != last_address) {
            last_address = RootHubDev.bAddress;
        }
    }

    if (RootHubDev.bType == USB_DEV_CLASS_HID) {
        process_device_interfaces (0);
    } else if (RootHubDev.bType == USB_DEV_CLASS_HUB) {
        for (uint8_t device = 1; device < MAX_HUB_DEVICES; device++) {
            process_device_interfaces (device);
        }
    }
}
