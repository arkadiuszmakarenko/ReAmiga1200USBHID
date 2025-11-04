# Independent Makefile for CH32V20x Bootloader
# All build artifacts will be placed in the /build directory

#############################
# Project configuration
#############################

# Project name
PROJECT = Bootloader

# Build type: Release (size-optimized) or Debug (debuggable)
# Override on CLI: make BUILD_TYPE=Debug
BUILD_TYPE ?= Release

# Build directory (separate per build type)
ifeq ($(BUILD_TYPE),Debug)
BUILD_DIR = build-debug
else
BUILD_DIR = build
endif

# Source directories
SRC_DIRS = Core Debug Peripheral/src Startup User User/Host_IAP User/Pff User/USB_Host

# Include directories
INC_DIRS = -IDebug \
           -ICore \
           -IUser \
           -IPeripheral/inc \
           -IUser/Host_IAP \
           -IUser/USB_Host \
           -IUser/Pff

# Toolchain Configuration
# Default to MRS toolchain, but allow customization via environment variables
MRS_TOOLCHAIN_ROOT ?= ../../MRS_Toolchain_Linux_x64_V210
TOOLCHAIN_VERSION ?= RISC-V Embedded GCC12

# Auto-detect toolchain prefix based on version
ifeq ($(TOOLCHAIN_VERSION),RISC-V Embedded GCC12)
    TOOLCHAIN_PREFIX ?= riscv-wch-elf
else ifeq ($(TOOLCHAIN_VERSION),RISC-V Embedded GCC)
    TOOLCHAIN_PREFIX ?= riscv-none-embed
else
    TOOLCHAIN_PREFIX ?= riscv-wch-elf
endif

# Toolchain paths (properly handle spaces in path)
TOOLCHAIN_BIN_DIR := $(MRS_TOOLCHAIN_ROOT)/$(TOOLCHAIN_VERSION)/bin
TOOLCHAIN_PATH := $(TOOLCHAIN_BIN_DIR)/$(TOOLCHAIN_PREFIX)

# Toolchain executables
CC := $(TOOLCHAIN_PATH)-gcc
CXX := $(TOOLCHAIN_PATH)-g++
AS := $(TOOLCHAIN_PATH)-as
OBJCOPY := $(TOOLCHAIN_PATH)-objcopy
OBJDUMP := $(TOOLCHAIN_PATH)-objdump
SIZE := $(TOOLCHAIN_PATH)-size
AR := $(TOOLCHAIN_PATH)-ar
GDB := $(TOOLCHAIN_PATH)-gdb
READELF := $(TOOLCHAIN_PATH)-readelf

# Alternative toolchain option - set USE_SYSTEM_TOOLCHAIN=1 to use system toolchain
ifeq ($(USE_SYSTEM_TOOLCHAIN),1)
CC := riscv64-unknown-elf-gcc
CXX := riscv64-unknown-elf-g++
AS := riscv64-unknown-elf-as
OBJCOPY := riscv64-unknown-elf-objcopy
OBJDUMP := riscv64-unknown-elf-objdump
SIZE := riscv64-unknown-elf-size
AR := riscv64-unknown-elf-ar
GDB := riscv64-unknown-elf-gdb
READELF := riscv64-unknown-elf-readelf
endif

# Base compiler flags
CFLAGS = -march=rv32imac_zicsr \
         -mabi=ilp32 \
         -msmall-data-limit=0 \
         -msave-restore \
         -fmax-errors=20 \
         -fmessage-length=0 \
         -fsigned-char \
         -ffunction-sections \
         -fdata-sections \
         -fno-common \
         -fno-builtin \
         -fno-unwind-tables \
         -fno-asynchronous-unwind-tables \
         -fno-stack-protector \
         -fmerge-all-constants \
         -Wunused \
         -Wuninitialized \
         -std=gnu99 \
         -MMD \
         -MP

# Assembler flags
ASFLAGS = -march=rv32imac_zicsr \
          -mabi=ilp32 \
          -msmall-data-limit=0 \
          -msave-restore \
          -fmax-errors=20 \
          -fmessage-length=0 \
          -fsigned-char \
          -ffunction-sections \
          -fdata-sections \
          -fno-common \
          -Wunused \
          -Wuninitialized \
          -x assembler-with-cpp \
          -IStartup \
          -MMD \
          -MP

# Base linker flags
LDFLAGS = -march=rv32imac_zicsr \
          -mabi=ilp32 \
          -msmall-data-limit=0 \
          -msave-restore \
          -fmax-errors=20 \
          -fmessage-length=0 \
          -fsigned-char \
          -ffunction-sections \
          -fdata-sections \
          -fno-common \
          -fno-builtin \
          -fno-unwind-tables \
          -fno-asynchronous-unwind-tables \
          -fno-stack-protector \
          -fmerge-all-constants \
          -Wunused \
          -Wuninitialized \
          -TLd/Link.ld \
          -nostartfiles \
          -Xlinker --gc-sections \
          -Xlinker --print-memory-usage \
          -Wl,-Map,$(BUILD_DIR)/$(PROJECT).map \
					-Wl,--relax \
          --specs=nano.specs \
          --specs=nosys.specs

#############################
# Optimization and debug per build type
#############################

ifeq ($(BUILD_TYPE),Debug)
	# Maximum debugging experience - all debug flags enabled
	CFLAGS += -Og -g3 -ggdb3 -gdwarf-4 \
	          -fno-omit-frame-pointer \
	          -fvar-tracking \
	          -fvar-tracking-assignments \
	          -fno-eliminate-unused-debug-types \
	          -fno-inline \
	          -fno-inline-small-functions \
	          -fno-inline-functions-called-once \
	          -fdebug-prefix-map=$(shell pwd)=. \
	          -grecord-gcc-switches
	ASFLAGS += -g3 -ggdb3 -gdwarf-4
	LDFLAGS += -g3 -ggdb3
	# Avoid LTO in debug for clearer stepping and symbols
	# No strip of symbols
else
	# Size-optimized release
	CFLAGS += -Os -flto -fno-inline-functions-called-once -fno-tree-loop-optimize -fno-jump-tables
	ASFLAGS += -Os
	LDFLAGS += -Os -flto -Wl,--strip-all
endif

# Find all C source files
C_SOURCES = $(shell find Core Debug Peripheral/src User -name "*.c")

# Find all assembly source files (only use the D6 startup file as per original build)
ASM_SOURCES = Startup/startup_ch32v20x_D6.S

# Generate object file names in build directory
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:%.S=$(BUILD_DIR)/%.o)
ALL_OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Generate dependency file names
C_DEPS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.d)
ASM_DEPS = $(ASM_SOURCES:%.S=$(BUILD_DIR)/%.d)
ALL_DEPS = $(C_DEPS) $(ASM_DEPS)

# Default target
all: check-toolchain $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).lst size

# Check if toolchain is available
check-toolchain:
	@echo "Checking toolchain..."
	@if [ ! -f "$(CC)" ]; then \
		echo "Error: Compiler not found at $(CC)"; \
		echo "Please check your toolchain configuration or run 'make toolchain-info'"; \
		echo "You can also set USE_SYSTEM_TOOLCHAIN=1 to use system toolchain"; \
		exit 1; \
	fi
	@echo "✓ Using toolchain: $(CC)"

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Create subdirectories in build folder
$(BUILD_DIR)/%/:
	@mkdir -p $@

# Rule for C source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@"$(CC)" $(CFLAGS) $(INC_DIRS) -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"

# Rule for assembly source files
$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling $<"
	@"$(CC)" $(ASFLAGS) -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"

# Rule for linking the ELF file
$(BUILD_DIR)/$(PROJECT).elf: $(ALL_OBJECTS) | $(BUILD_DIR)
	@echo "Linking $@"
	@"$(CC)" $(LDFLAGS) -o "$@" $(ALL_OBJECTS)

# Rule for creating hex file
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Creating $@"
	@"$(OBJCOPY)" -O ihex "$<" "$@"

# Rule for creating listing file
$(BUILD_DIR)/$(PROJECT).lst: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Creating $@"
	@"$(OBJDUMP)" --all-headers --demangle --disassemble -M xw "$<" > "$@"

# Rule for creating binary file
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Creating $@"
	@"$(OBJCOPY)" -O binary "$<" "$@"

# Show size information
size: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Size information:"
	@"$(SIZE)" --format=berkeley "$<"

# Show detailed memory usage
memory-usage: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Detailed memory usage:"
	@"$(SIZE)" --format=sysv "$<"
	@echo ""
	@echo "Section sizes:"
	@"$(READELF)" -S "$<" | grep -E '(\.text|\.data|\.bss|\.rodata)'

# Flash target (requires OpenOCD configuration)
flash: $(BUILD_DIR)/$(PROJECT).hex
	@echo "Flashing $(PROJECT).hex to device..."
	@if [ -f "$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd" ]; then \
		echo "Using MRS OpenOCD..."; \
		cd $(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin && ./openocd -f wch-riscv.cfg -c "program $(shell pwd)/$(BUILD_DIR)/$(PROJECT).hex verify reset exit"; \
	else \
		echo "MRS OpenOCD not found. Please configure your programming tool."; \
		echo "You can flash manually using: $(BUILD_DIR)/$(PROJECT).hex"; \
	fi

# Flash binary file (alternative method)
flash-bin: $(BUILD_DIR)/$(PROJECT).bin
	@echo "Flashing $(PROJECT).bin to device..."
	@if [ -f "$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd" ]; then \
		echo "Using MRS OpenOCD..."; \
		cd $(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin && ./openocd -f wch-riscv.cfg -c "program $(shell pwd)/$(BUILD_DIR)/$(PROJECT).bin verify reset exit 0x00000000"; \
	else \
		echo "MRS OpenOCD not found. Please configure your programming tool."; \
		echo "You can flash manually using: $(BUILD_DIR)/$(PROJECT).bin"; \
	fi

# Debug target
debug: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Starting GDB debug session..."
	@echo "Connect to target first, then run: target remote localhost:3333"
	@"$(GDB)" "$<"

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# Create build directory structure
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

# Show help
help:
	@echo "CH32V20x Bootloader Makefile"
	@echo "=============================="
	@echo ""
	@echo "Available targets:"
	@echo "  all            - Build the project (default)"
	@echo "  clean          - Remove build directory and all artifacts"
	@echo "  dirs           - Create build directory structure"
	@echo "  size           - Show size information for the ELF file"
	@echo "  memory-usage   - Show detailed memory usage breakdown"
	@echo "  flash          - Flash the hex file to device (requires OpenOCD)"
	@echo "  debug          - Start GDB debug session"
	@echo "  toolchain-info - Show current toolchain configuration"
	@echo "  toolchain-env  - Show environment setup for terminal usage"
	@echo "  help           - Show this help message"
	@echo ""
	@echo "Toolchain Customization:"
	@echo "  MRS_TOOLCHAIN_ROOT  - Path to MRS toolchain (default: ../../MRS_Toolchain_Linux_x64_V210)"
	@echo "  TOOLCHAIN_VERSION   - Toolchain version (default: RISC-V Embedded GCC12)"
	@echo "                        Options: 'RISC-V Embedded GCC12' or 'RISC-V Embedded GCC'"
	@echo "  TOOLCHAIN_PREFIX    - Tool prefix (default: riscv-wch-elf)"
	@echo "  USE_SYSTEM_TOOLCHAIN=1 - Use system toolchain instead of MRS"
	@echo "  BUILD_TYPE={Release|Debug} - Select build profile (default: Release)"
	@echo ""
	@echo "Examples:"
	@echo "  make                                    # Release build (size-optimized)"
	@echo "  make BUILD_TYPE=Debug                   # Debug build with symbols"
	@echo "  make TOOLCHAIN_VERSION='RISC-V Embedded GCC'  # Use older GCC version"
	@echo "  make USE_SYSTEM_TOOLCHAIN=1            # Use system toolchain"
	@echo "  make MRS_TOOLCHAIN_ROOT=/custom/path   # Use custom toolchain path"
	@echo ""
	@echo "Build artifacts will be created in: $(BUILD_DIR)/"
	@echo "  $(PROJECT).elf - Executable file"
	@echo "  $(PROJECT).hex - Intel HEX file for flashing"
	@echo "  $(PROJECT).bin - Binary file for programming"
	@echo "  $(PROJECT).lst - Disassembly listing"
	@echo "  $(PROJECT).map - Memory map file"

# Include dependency files if they exist
-include $(ALL_DEPS)

# Show toolchain information
toolchain-info:
	@echo "Toolchain Configuration:"
	@echo "========================"
	@echo "MRS_TOOLCHAIN_ROOT: $(MRS_TOOLCHAIN_ROOT)"
	@echo "TOOLCHAIN_VERSION:  $(TOOLCHAIN_VERSION)"
	@echo "TOOLCHAIN_PREFIX:   $(TOOLCHAIN_PREFIX)"
	@echo "TOOLCHAIN_BIN_DIR:  $(TOOLCHAIN_BIN_DIR)"
	@echo "USE_SYSTEM_TOOLCHAIN: $(USE_SYSTEM_TOOLCHAIN)"
	@echo ""
	@echo "Active Tools:"
	@echo "CC:      $(CC)"
	@echo "CXX:     $(CXX)"
	@echo "OBJCOPY: $(OBJCOPY)"
	@echo "OBJDUMP: $(OBJDUMP)"
	@echo "SIZE:    $(SIZE)"
	@echo "GDB:     $(GDB)"
	@echo ""
	@echo "Checking toolchain availability..."
	@if [ -f "$(CC)" ]; then \
		echo "✓ Compiler found: $(CC)"; \
		"$(CC)" --version | head -1; \
	else \
		echo "✗ Compiler not found: $(CC)"; \
		echo "  Please check your toolchain configuration"; \
	fi

# Show environment setup for using the toolchain in terminal
toolchain-env:
	@echo "Environment setup for using MRS toolchain in terminal:"
	@echo "======================================================"
	@echo ""
	@echo "Add this to your PATH to use the toolchain directly:"
	@echo "export PATH=\"$(shell pwd)/$(TOOLCHAIN_BIN_DIR):\$$PATH\""
	@echo ""
	@echo "Or run this command in your current shell:"
	@echo "PATH=\"$(shell pwd)/$(TOOLCHAIN_BIN_DIR):\$$PATH\""
	@echo ""
	@echo "Then you can use commands like:"
	@echo "  $(TOOLCHAIN_PREFIX)-gcc --version"
	@echo "  $(TOOLCHAIN_PREFIX)-objdump --help"

# Phony targets
.PHONY: all clean dirs size memory-usage flash debug help toolchain-info toolchain-env debug-vars check-toolchain

# Print variables for debugging
debug-vars:
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "ASM_SOURCES: $(ASM_SOURCES)"
	@echo "C_OBJECTS: $(C_OBJECTS)"
	@echo "ASM_OBJECTS: $(ASM_OBJECTS)"
	@echo "BUILD_DIR: $(BUILD_DIR)"