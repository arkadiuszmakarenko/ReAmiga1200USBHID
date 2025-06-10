################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/core_riscv.c 

C_DEPS += \
./Core/core_riscv.d 

OBJS += \
./Core/core_riscv.o 



# Each subdirectory must supply rules for building sources it contributes
Core/%.o: ../Core/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/Debug" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/User/USB_Host" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/Core" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/User" -I"/home/makaron/Repo/ReAmiga1200USBHID/firmware/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
