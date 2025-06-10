################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/USB_Host/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
A1200HID_USB.bin \
A1200HID_USB.hex \

SECONDARY_LIST += \
A1200HID_USB.lst \

SECONDARY_SIZE += \
A1200HID_USB.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: A1200HID_USB.elf secondary-outputs

# Tool invocations
A1200HID_USB.elf: $(OBJS) $(USER_OBJS)
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "/home/makaron/Repo/ReAmiga1200USBHID/firmware/Ld/Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"A1200HID_USB.map" --specs=nano.specs --specs=nosys.specs -o "A1200HID_USB.elf" $(OBJS) $(USER_OBJS) $(LIBS)
A1200HID_USB.bin: A1200HID_USB.elf
	@	riscv-none-embed-objcopy -O binary "A1200HID_USB.elf" "A1200HID_USB.bin"
A1200HID_USB.hex: A1200HID_USB.elf
	@	riscv-none-embed-objcopy -O ihex "A1200HID_USB.elf" "A1200HID_USB.hex"
A1200HID_USB.lst: A1200HID_USB.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "A1200HID_USB.elf" > "A1200HID_USB.lst"
A1200HID_USB.siz: A1200HID_USB.elf
	riscv-none-embed-size --format=berkeley "A1200HID_USB.elf"

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(S_DEPS)$(S_UPPER_DEPS)$(ASM_DEPS)$(ASM_UPPER_DEPS)$(C_DEPS) A1200HID_USB.elf

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets