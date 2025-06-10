################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/USB_Host/app_km.c \
../User/USB_Host/ch32v20x_usbfs_host.c \
../User/USB_Host/usb_gamepad.c \
../User/USB_Host/usb_hid_reportparser.c \
../User/USB_Host/usb_host_hid.c \
../User/USB_Host/usb_host_hub.c \
../User/USB_Host/usb_keyboard.c \
../User/USB_Host/usb_mouse.c 

C_DEPS += \
./User/USB_Host/app_km.d \
./User/USB_Host/ch32v20x_usbfs_host.d \
./User/USB_Host/usb_gamepad.d \
./User/USB_Host/usb_hid_reportparser.d \
./User/USB_Host/usb_host_hid.d \
./User/USB_Host/usb_host_hub.d \
./User/USB_Host/usb_keyboard.d \
./User/USB_Host/usb_mouse.d 

OBJS += \
./User/USB_Host/app_km.o \
./User/USB_Host/ch32v20x_usbfs_host.o \
./User/USB_Host/usb_gamepad.o \
./User/USB_Host/usb_hid_reportparser.o \
./User/USB_Host/usb_host_hid.o \
./User/USB_Host/usb_host_hub.o \
./User/USB_Host/usb_keyboard.o \
./User/USB_Host/usb_mouse.o 



# Each subdirectory must supply rules for building sources it contributes
User/USB_Host/%.o: ../User/USB_Host/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/Debug" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/User/USB_Host" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/Core" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/User" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
