ReAmiga1200USBHID 

![PCB](https://github.com/arkadiuszmakarenko/ReAmiga1200USBHID/assets/5903872/26648a14-64e9-4a42-a82d-14dc1d464c64)

![PXL_20240316_200650190 MP](https://github.com/arkadiuszmakarenko/ReAmiga1200USBHID/assets/5903872/a914649a-9c0c-4c13-80dd-66ac01cf9a3e)


Mouse Driver
Mouse driver has been provided by sq7bti.

I have used following other projects
- Mist (USB HID parser stack)
- Simon Inns smallymouse2 mouse quadrature encoding
- Amiga Keyboard handler: https://github.com/gianlucarenzi/stalker


This device supports following
- Mouse including scroll functionality (it supports 2 buttons only without a driver)
- Gamepad
- Keyboards

USB supports 4 port Hubs. Only good quality hubs work correctly.

WCHISPTool_CMD 
Application download address: https://www.wch.cn/downloads/WCHISPTool_Setup_exe.html
Windows has GUI application.

- Set up jumper for boot next to serial jumpers to allow serial bootloader.
- Power on board.
- Set up application to program chip CH32V203C8U6
- Set up COM port used for programming
- Set up Object file 1 for firmware hex file.
- Deprotect the chip.
- Download firmware to the chip.


**This project is open source.**
