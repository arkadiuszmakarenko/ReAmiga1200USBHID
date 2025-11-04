# Unified Makefile for RISKYREA1200 Projects
# Supports three build variants:
#   - Bootloader: Custom bootloader (FLASH @ 0x00000000, src in Bootloader/)
#   - RISKYREA1200: Standalone application (FLASH @ 0x00000000, src in src/)
#   - RISKYREA1200Bootloader: Application for custom bootloader (FLASH @ 0x00003000, src in src/)
# Main applications share source code from src/ directory

#############################
# Project configuration
#############################

# Project variant: Bootloader, RISKYREA1200, or RISKYREA1200Bootloader
# Override on CLI: make PROJECT=RISKYREA1200Bootloader
PROJECT ?= RISKYREA1200

# Build type: Release (size-optimized) or Debug (debuggable)
# Override on CLI: make BUILD_TYPE=Debug
BUILD_TYPE ?= Release

# Validate PROJECT selection
ifeq ($(filter $(PROJECT),Bootloader RISKYREA1200 RISKYREA1200Bootloader),)
    $(error Invalid PROJECT: $(PROJECT). Must be Bootloader, RISKYREA1200, or RISKYREA1200Bootloader)
endif

# Build directory (separate per project and build type)
BUILD_DIR = build/$(PROJECT)/$(BUILD_TYPE)

# Source directory selection
ifeq ($(PROJECT),Bootloader)
    SRC_DIR = Bootloader
    SRC_SUBDIRS = Core Debug Peripheral/src Startup User User/Host_IAP User/Pff User/USB_Host
else
    SRC_DIR = src
    SRC_SUBDIRS = Core Debug Peripheral/src Startup User User/USB_Host
endif

# Include directories
ifeq ($(PROJECT),Bootloader)
    INC_DIRS = -I$(SRC_DIR)/Debug \
               -I$(SRC_DIR)/Core \
               -I$(SRC_DIR)/User \
               -I$(SRC_DIR)/Peripheral/inc \
               -I$(SRC_DIR)/User/Host_IAP \
               -I$(SRC_DIR)/User/USB_Host \
               -I$(SRC_DIR)/User/Pff
else
    INC_DIRS = -I$(SRC_DIR)/Debug \
               -I$(SRC_DIR)/Core \
               -I$(SRC_DIR)/User \
               -I$(SRC_DIR)/Peripheral/inc \
               -I$(SRC_DIR)/User/USB_Host
endif

# Linker script selection
ifeq ($(PROJECT),Bootloader)
    LINKER_SCRIPT = $(SRC_DIR)/Ld/Link.ld
else ifeq ($(PROJECT),RISKYREA1200Bootloader)
    LINKER_SCRIPT = RISKYREA1200Boorloader/Ld/Link.ld
else
    LINKER_SCRIPT = RISKYREA1200/Ld/Link.ld
endif

# Toolchain Configuration
# Default to MRS toolchain, but allow customization via environment variables
MRS_TOOLCHAIN_ROOT ?= ../MRS_Toolchain_Linux_x64_V210
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
          -I$(SRC_DIR)/Startup \
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
          -T$(LINKER_SCRIPT) \
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
ifeq ($(PROJECT),Bootloader)
    C_SOURCES = $(shell find $(SRC_DIR)/Core $(SRC_DIR)/Debug $(SRC_DIR)/Peripheral/src $(SRC_DIR)/User -name "*.c")
else
    C_SOURCES = $(shell find $(SRC_DIR)/Core $(SRC_DIR)/Debug $(SRC_DIR)/Peripheral/src $(SRC_DIR)/User -name "*.c")
endif

# Find all assembly source files (only use the D6 startup file)
ASM_SOURCES = $(SRC_DIR)/Startup/startup_ch32v20x_D6.S

# Generate object file names in build directory
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
ALL_OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Generate dependency file names
C_DEPS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(C_SOURCES))
ASM_DEPS = $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.d,$(ASM_SOURCES))
ALL_DEPS = $(C_DEPS) $(ASM_DEPS)

# Default target
all: check-toolchain $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).lst size post-build

# Check if toolchain is available
check-toolchain:
	@echo "================================"
	@echo "Building: $(PROJECT)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Linker Script: $(LINKER_SCRIPT)"
	@echo "================================"
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

# Rule for C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@"$(CC)" $(CFLAGS) $(INC_DIRS) -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"

# Rule for assembly source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S | $(BUILD_DIR)
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

# Post-build step: create RISKYREA.UPD for bootloader variant
post-build:
ifeq ($(PROJECT),RISKYREA1200Bootloader)
	@echo "Creating RISKYREA.UPD for bootloader update..."
	@cp "$(BUILD_DIR)/$(PROJECT).bin" "$(BUILD_DIR)/RISKYREA.UPD"
	@echo "✓ Created: $(BUILD_DIR)/RISKYREA.UPD"
endif

# Combine bootloader and application into a single release binary
combine-release:
	@echo "================================"
	@echo "Creating combined release binary"
	@echo "================================"
	@if [ ! -f "build/Bootloader/Debug/Bootloader.bin" ]; then \
		echo "Error: Bootloader binary not found. Build it first:"; \
		echo "  make PROJECT=Bootloader BUILD_TYPE=Debug"; \
		exit 1; \
	fi
	@if [ ! -f "build/RISKYREA1200Bootloader/Debug/RISKYREA1200Bootloader.bin" ]; then \
		echo "Error: RISKYREA1200Bootloader binary not found. Build it first:"; \
		echo "  make PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Debug"; \
		exit 1; \
	fi
	@echo "Expanding Bootloader.bin to 12KB (0x3000 bytes)..."
	@dd if=build/Bootloader/Debug/Bootloader.bin of=build/Bootloader_padded.bin bs=1 count=12288 conv=sync 2>/dev/null
	@truncate -s 12288 build/Bootloader_padded.bin
	@echo "Expanding RISKYREA1200Bootloader.bin to 52KB (0xD000 bytes)..."
	@dd if=build/RISKYREA1200Bootloader/Debug/RISKYREA1200Bootloader.bin of=build/App_padded.bin bs=1 count=53248 conv=sync 2>/dev/null
	@truncate -s 53248 build/App_padded.bin
	@echo "Combining binaries..."
	@cat build/Bootloader_padded.bin build/App_padded.bin > build/RISKYREA1200Release.bin
	@rm -f build/Bootloader_padded.bin build/App_padded.bin
	@echo "✓ Created: build/RISKYREA1200Release.bin"
	@ls -lh build/RISKYREA1200Release.bin
	@echo ""
	@echo "Flash layout:"
	@echo "  0x00000000 - 0x00002FFF : Bootloader (12KB)"
	@echo "  0x00003000 - 0x0000FFFF : Application (52KB)"
	@echo "  Total: 64KB"

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

# Flash target - flashes combined release binary (bootloader + application)
# Uses sudo to access USB device - WCH-Link requires root permissions
flash: build/RISKYREA1200Release.bin
	@echo "Flashing RISKYREA1200Release.bin (64KB: Bootloader + Application) to device..."
	@if [ ! -f "$<" ]; then \
		echo "Combined binary not found. Building..."; \
		$(MAKE) release-combined; \
	fi
	@if [ -f "$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd" ]; then \
		echo "Using MRS OpenOCD with sudo (required for USB access)..."; \
		sudo $(shell pwd)/$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd \
			-f $(shell pwd)/$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/wch-riscv.cfg \
			-c "program $(shell pwd)/$< verify reset exit 0x00000000"; \
	else \
		echo "MRS OpenOCD not found. Please configure your programming tool."; \
		echo "You can flash manually using: $<"; \
	fi

# Flash binary file (alternative method for individual project binaries)
flash-bin: $(BUILD_DIR)/$(PROJECT).bin
	@echo "Flashing $(PROJECT).bin to device..."
	@if [ -f "$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd" ]; then \
		echo "Using MRS OpenOCD..."; \
		cd $(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin && ./openocd -f wch-riscv.cfg -c "program $(shell pwd)/$(BUILD_DIR)/$(PROJECT).bin verify reset exit 0x00000000"; \
	else \
		echo "MRS OpenOCD not found. Please configure your programming tool."; \
		echo "You can flash manually using: $(BUILD_DIR)/$(PROJECT).bin"; \
	fi

# Erase chip - full flash erase
erase:
	@echo "Erasing entire flash chip..."
	@if [ -f "$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd" ]; then \
		echo "Using MRS OpenOCD with sudo..."; \
		sudo $(shell pwd)/$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd \
			-f $(shell pwd)/$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/wch-riscv.cfg \
			-c "init" \
			-c "halt" \
			-c "flash erase_sector 0 0 last" \
			-c "reset" \
			-c "shutdown"; \
	else \
		echo "MRS OpenOCD not found. Please configure your programming tool."; \
	fi

# Debug target
debug: $(BUILD_DIR)/$(PROJECT).elf
	@echo "Starting GDB debug session..."
	@echo "Connect to target first, then run: target remote localhost:3333"
	@"$(GDB)" "$<"

# Clean build artifacts for current project
clean:
	@echo "Cleaning $(PROJECT) $(BUILD_TYPE) build directory..."
	@rm -rf $(BUILD_DIR)

# Clean all build artifacts (all projects and build types)
clean-all:
	@echo "Cleaning all build directories..."
	@rm -rf build

# Create build directory structure
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(addprefix $(BUILD_DIR)/,$(SRC_SUBDIRS))

# Build both project variants
both: both-release

# Build all three variants in Release mode
all-release:
	@echo "Building all project variants in Release mode..."
	@$(MAKE) PROJECT=Bootloader BUILD_TYPE=Release
	@$(MAKE) PROJECT=RISKYREA1200 BUILD_TYPE=Release
	@$(MAKE) PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Release
	@$(MAKE) combine-release

# Build all three variants in Debug mode
all-debug:
	@echo "Building all project variants in Debug mode..."
	@$(MAKE) PROJECT=Bootloader BUILD_TYPE=Debug
	@$(MAKE) PROJECT=RISKYREA1200 BUILD_TYPE=Debug
	@$(MAKE) PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Debug
	@$(MAKE) combine-release

# Build combined release binary (bootloader + application)
release-combined: 
	@echo "Building combined release binary..."
	@$(MAKE) PROJECT=Bootloader BUILD_TYPE=Debug
	@$(MAKE) PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Debug
	@$(MAKE) combine-release

# Build both main app variants (backward compatibility)
both-release:
	@echo "Building both main application variants in Release mode..."
	@$(MAKE) PROJECT=RISKYREA1200 BUILD_TYPE=Release
	@$(MAKE) PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Release

# Build both main app variants in Debug mode (backward compatibility)
both-debug:
	@echo "Building both main application variants in Debug mode..."
	@$(MAKE) PROJECT=RISKYREA1200 BUILD_TYPE=Debug
	@$(MAKE) PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Debug

# Show help
help:
	@echo "RISKYREA1200 Unified Makefile"
	@echo "=============================="
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build the selected project (default)"
	@echo "  clean            - Remove build artifacts for current project/build type"
	@echo "  clean-all        - Remove all build artifacts (all projects)"
	@echo "  dirs             - Create build directory structure"
	@echo "  size             - Show size information for the ELF file"
	@echo "  memory-usage     - Show detailed memory usage breakdown"
	@echo "  flash            - Flash combined release binary to device (requires OpenOCD)"
	@echo "  erase            - Erase entire flash chip (requires OpenOCD)"
	@echo "  debug            - Start GDB debug session"
	@echo "  both             - Build both main app variants (Release)"
	@echo "  both-release     - Build RISKYREA1200 + RISKYREA1200Bootloader (Release)"
	@echo "  both-debug       - Build RISKYREA1200 + RISKYREA1200Bootloader (Debug)"
	@echo "  all-release      - Build all three variants (Bootloader + apps, Release)"
	@echo "  all-debug        - Build all three variants (Bootloader + apps, Debug)"
	@echo "  release-combined - Build Bootloader + RISKYREA1200Bootloader and combine into single binary"
	@echo "  combine-release  - Combine existing Bootloader + RISKYREA1200Bootloader binaries"
	@echo "  toolchain-info   - Show current toolchain configuration"
	@echo "  toolchain-env    - Show environment setup for terminal usage"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Project Selection:"
	@echo "  PROJECT=Bootloader              - Custom bootloader (FLASH @ 0x00000000, 64K)"
	@echo "  PROJECT=RISKYREA1200            - Standalone application (FLASH @ 0x00000000, 64K)"
	@echo "  PROJECT=RISKYREA1200Bootloader  - Bootloader application (FLASH @ 0x00003000, 54K)"
	@echo ""
	@echo "Build Type Selection:"
	@echo "  BUILD_TYPE=Release  - Size-optimized build (default)"
	@echo "  BUILD_TYPE=Debug    - Debug build with full symbols"
	@echo ""
	@echo "Toolchain Customization:"
	@echo "  MRS_TOOLCHAIN_ROOT  - Path to MRS toolchain (default: ../MRS_Toolchain_Linux_x64_V210)"
	@echo "  TOOLCHAIN_VERSION   - Toolchain version (default: RISC-V Embedded GCC12)"
	@echo "                        Options: 'RISC-V Embedded GCC12' or 'RISC-V Embedded GCC'"
	@echo "  TOOLCHAIN_PREFIX    - Tool prefix (default: riscv-wch-elf)"
	@echo "  USE_SYSTEM_TOOLCHAIN=1 - Use system toolchain instead of MRS"
	@echo ""
	@echo "Examples:"
	@echo "  make                                           # Build RISKYREA1200 Release"
	@echo "  make PROJECT=Bootloader BUILD_TYPE=Debug      # Build bootloader Debug"
	@echo "  make PROJECT=RISKYREA1200Bootloader            # Build bootloader variant Release"
	@echo "  make PROJECT=RISKYREA1200 BUILD_TYPE=Debug    # Build standalone Debug"
	@echo "  make all-debug                                 # Build all three variants in Debug"
	@echo "  make both-debug                                # Build both main apps in Debug"
	@echo "  make release-combined                          # Build and combine bootloader + app"
	@echo "  make clean-all                                 # Clean all builds"
	@echo ""
	@echo "Build artifacts location: build/<PROJECT>/<BUILD_TYPE>/"
	@echo "  Example: build/Bootloader/Debug/Bootloader.elf"
	@echo "  Example: build/RISKYREA1200/Debug/RISKYREA1200.elf"
	@echo "  Example: build/RISKYREA1200Bootloader/Release/RISKYREA1200Bootloader.hex"

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
.PHONY: all clean clean-all dirs size memory-usage flash flash-bin erase debug help toolchain-info toolchain-env \
        check-toolchain both both-release both-debug all-release all-debug post-build \
        combine-release release-combined

# Print variables for debugging
debug-vars:
	@echo "PROJECT: $(PROJECT)"
	@echo "BUILD_TYPE: $(BUILD_TYPE)"
	@echo "BUILD_DIR: $(BUILD_DIR)"
	@echo "LINKER_SCRIPT: $(LINKER_SCRIPT)"
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "ASM_SOURCES: $(ASM_SOURCES)"
	@echo "C_OBJECTS: $(C_OBJECTS)"
	@echo "ASM_OBJECTS: $(ASM_OBJECTS)"
