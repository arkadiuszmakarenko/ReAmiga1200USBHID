# Unified Makefile for RISKYREA1200 Projects
# Supports three build variants:
#   - Bootloader: Custom bootloader (FLASH @ 0x00000000, src in Bootloader/)
#   - RISKYREA1200: Standalone application (FLASH @ 0x00000000, src in src/)
#   - RISKYREA1200Bootloader: Application for custom bootloader (FLASH @ 0x00004000, src in src/)
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
# Can be overridden via environment or command line for containerized builds
MRS_TOOLCHAIN_ROOT ?= ../MRS_Toolchain_Linux_x64_V210
TOOLCHAIN_PREFIX ?= riscv-wch-elf

# Toolchain paths - can specify full path to toolchain bin directory
# For containers, set TOOLCHAIN_PATH to the bin directory, e.g.:
#   make TOOLCHAIN_PATH=/opt/riscv/bin/riscv-wch-elf
ifneq ($(TOOLCHAIN_PATH),)
    # User provided full toolchain path
    CC := $(TOOLCHAIN_PATH)-gcc
    OBJCOPY := $(TOOLCHAIN_PATH)-objcopy
    OBJDUMP := $(TOOLCHAIN_PATH)-objdump
    SIZE := $(TOOLCHAIN_PATH)-size
    GDB := $(TOOLCHAIN_PATH)-gdb
    READELF := $(TOOLCHAIN_PATH)-readelf
else
    # Use MRS toolchain root
    TOOLCHAIN_BIN_DIR := $(MRS_TOOLCHAIN_ROOT)/RISC-V Embedded GCC12/bin
    TOOLCHAIN_PATH := $(TOOLCHAIN_BIN_DIR)/$(TOOLCHAIN_PREFIX)
    CC := $(TOOLCHAIN_PATH)-gcc
    OBJCOPY := $(TOOLCHAIN_PATH)-objcopy
    OBJDUMP := $(TOOLCHAIN_PATH)-objdump
    SIZE := $(TOOLCHAIN_PATH)-size
    GDB := $(TOOLCHAIN_PATH)-gdb
    READELF := $(TOOLCHAIN_PATH)-readelf
endif

# Base compiler flags
CFLAGS = -march=rv32imacxw \
         -mabi=ilp32 \
         -msmall-data-limit=8 \
         -msave-restore \
         -fmax-errors=20 \
         -fmessage-length=0 \
         -fsigned-char \
         -ffunction-sections \
         -fdata-sections \
         -fno-common \
         -Wunused \
         -Wuninitialized \
         -std=gnu99 \
         -MMD \
         -MP

# Assembler flags
ASFLAGS = -march=rv32imacxw \
          -mabi=ilp32 \
          -msmall-data-limit=8 \
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
LDFLAGS = -march=rv32imacxw \
          -mabi=ilp32 \
          -msmall-data-limit=8 \
          -msave-restore \
          -fmax-errors=20 \
          -fmessage-length=0 \
          -fsigned-char \
          -ffunction-sections \
          -fdata-sections \
          -fno-common \
          -Wunused \
          -Wuninitialized \
          -T$(LINKER_SCRIPT) \
          -nostartfiles \
          -Xlinker --gc-sections \
          -Wl,-Map,$(BUILD_DIR)/$(PROJECT).map \
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
	# Size-optimized release - match MRS toolchain settings
	CFLAGS += -Os -flto --param=highcode-gen-section-name=1
	ASFLAGS += -Os
	LDFLAGS += -Os -flto
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

# Default target - build all three projects in Release mode
all: all-release

# Build single project (used internally)
build-single: check-toolchain $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).lst size
	@$(MAKE) post-build

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
	@cp "$(BUILD_DIR)/$(PROJECT).bin" "build/RISKYREA.UPD"
	@echo "✓ Created: $(BUILD_DIR)/RISKYREA.UPD"
	@echo "✓ Created: build/RISKYREA.UPD"
endif

# Combine bootloader and application into a single release binary
# This target will build the required binaries if they don't exist
# Accepts COMBINE_BUILD_TYPE parameter (default: Debug for compatibility)
COMBINE_BUILD_TYPE ?= Debug

combine-release:
	@echo "================================"
	@echo "Creating combined release binary ($(COMBINE_BUILD_TYPE))"
	@echo "================================"
	@if [ ! -f "build/Bootloader/$(COMBINE_BUILD_TYPE)/Bootloader.bin" ]; then \
		echo "Building Bootloader ($(COMBINE_BUILD_TYPE))..."; \
		$(MAKE) build-single PROJECT=Bootloader BUILD_TYPE=$(COMBINE_BUILD_TYPE); \
	fi
	@if [ ! -f "build/RISKYREA1200Bootloader/$(COMBINE_BUILD_TYPE)/RISKYREA1200Bootloader.bin" ]; then \
		echo "Building RISKYREA1200Bootloader ($(COMBINE_BUILD_TYPE))..."; \
		$(MAKE) build-single PROJECT=RISKYREA1200Bootloader BUILD_TYPE=$(COMBINE_BUILD_TYPE); \
	fi
	@echo "Expanding Bootloader.bin to 16KB (0x4000 bytes)..."
	@dd if=build/Bootloader/$(COMBINE_BUILD_TYPE)/Bootloader.bin of=build/Bootloader_padded.bin bs=1 count=16384 conv=sync 2>/dev/null
	@truncate -s 16384 build/Bootloader_padded.bin
	@echo "Expanding RISKYREA1200Bootloader.bin to 48KB (0xC000 bytes)..."
	@dd if=build/RISKYREA1200Bootloader/$(COMBINE_BUILD_TYPE)/RISKYREA1200Bootloader.bin of=build/App_padded.bin bs=1 count=49152 conv=sync 2>/dev/null
	@truncate -s 49152 build/App_padded.bin
	@echo "Combining binaries..."
	@cat build/Bootloader_padded.bin build/App_padded.bin > build/RISKYREA1200Release.bin
	@rm -f build/Bootloader_padded.bin build/App_padded.bin
	@echo "✓ Created: build/RISKYREA1200Release.bin"
	@ls -lh build/RISKYREA1200Release.bin
	@echo ""
	@echo "Flash layout:"
	@echo "  0x00000000 - 0x00003FFF : Bootloader (16KB)"
	@echo "  0x00004000 - 0x0000FFFF : Application (48KB)"
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
flash: build/RISKYREA1200Release.bin
	@echo "Flashing RISKYREA1200Release.bin (64KB: Bootloader + Application) to device..."
	@if [ ! -f "$<" ]; then \
		echo "Combined binary not found. Building..."; \
		$(MAKE) all-release; \
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

# Flash bootloader only
flash-bootloader: build/Bootloader/Release/Bootloader.bin
	@echo "Flashing Bootloader only (16KB) to device at 0x00000000..."
	@if [ ! -f "$<" ]; then \
		echo "Bootloader binary not found. Building..."; \
		$(MAKE) build-single PROJECT=Bootloader BUILD_TYPE=Release; \
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

# Erase chip - full flash erase
erase:
	@echo "Erasing entire flash chip..."
	@sudo $(shell pwd)/$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/openocd \
		-f $(shell pwd)/$(MRS_TOOLCHAIN_ROOT)/OpenOCD/OpenOCD/bin/wch-riscv.cfg \
		-c "init" \
		-c "halt" \
		-c "flash erase_sector 0 0 last" \
		-c "reset" \
		-c "shutdown"

# Debug target
debug: $(BUILD_DIR)/$(PROJECT).elf
	@"$(GDB)" "$<"

# Clean all build artifacts
clean:
	@echo "Cleaning all build artifacts..."
	@rm -rf build
	@echo "✓ Cleaned everything"

# Alias for clean (backward compatibility)
clean-all: clean

# Build all three variants in Release mode
all-release:
	@echo "Building all project variants in Release mode..."
	@$(MAKE) build-single PROJECT=Bootloader BUILD_TYPE=Release
	@$(MAKE) build-single PROJECT=RISKYREA1200 BUILD_TYPE=Release
	@$(MAKE) build-single PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Release
	@$(MAKE) combine-release COMBINE_BUILD_TYPE=Release

# Build all three variants in Debug mode
all-debug:
	@echo "Building all project variants in Debug mode..."
	@$(MAKE) build-single PROJECT=Bootloader BUILD_TYPE=Debug
	@$(MAKE) build-single PROJECT=RISKYREA1200 BUILD_TYPE=Debug
	@$(MAKE) build-single PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Debug
	@$(MAKE) combine-release COMBINE_BUILD_TYPE=Debug

# Show help
help:
	@echo "RISKYREA1200 Unified Makefile"
	@echo "=============================="
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build all three variants in Release mode (default)"
	@echo "  all-release      - Build all three variants in Release mode"
	@echo "  all-debug        - Build all three variants in Debug mode"
	@echo "  build-single     - Build single selected project (use PROJECT= and BUILD_TYPE=)"
	@echo "  combine-release  - Combine Bootloader + RISKYREA1200Bootloader binaries"
	@echo "  flash            - Flash combined release binary to device"
	@echo "  flash-bootloader - Flash bootloader only to device"
	@echo "  erase            - Erase entire flash chip"
	@echo "  clean            - Remove all build artifacts"
	@echo "  size             - Show size information for the ELF file"
	@echo "  memory-usage     - Show detailed memory usage breakdown"
	@echo "  debug            - Start GDB debug session"
	@echo "  toolchain-info   - Show current toolchain configuration"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Project Selection:"
	@echo "  PROJECT=Bootloader              - Custom bootloader (FLASH @ 0x00000000, 64K)"
	@echo "  PROJECT=RISKYREA1200            - Standalone application (FLASH @ 0x00000000, 64K)"
	@echo "  PROJECT=RISKYREA1200Bootloader  - Bootloader application (FLASH @ 0x00004000, 48K)"
	@echo ""
	@echo "Build Type Selection:"
	@echo "  BUILD_TYPE=Release  - Size-optimized build (default)"
	@echo "  BUILD_TYPE=Debug    - Debug build with full symbols"
	@echo ""
	@echo "Toolchain Configuration:"
	@echo "  MRS_TOOLCHAIN_ROOT=/path/to/toolchain  - Override toolchain root directory"
	@echo "  TOOLCHAIN_PATH=/path/to/bin/prefix    - Override full toolchain path (for containers)"
	@echo "  TOOLCHAIN_PREFIX=prefix                - Override toolchain prefix (default: riscv-wch-elf)"
	@echo ""
	@echo "Examples:"
	@echo "  make                                           # Build all three variants in Release"
	@echo "  make PROJECT=Bootloader BUILD_TYPE=Debug      # Build bootloader Debug"
	@echo "  make all-debug                                 # Build all three variants in Debug"
	@echo "  make -j8                                       # Build with parallel compilation"
	@echo "  make flash                                     # Flash combined binary to device"
	@echo "  make clean                                     # Clean all builds"
	@echo ""
	@echo "Container/Custom Toolchain Examples:"
	@echo "  make TOOLCHAIN_PATH=/opt/riscv/bin/riscv-wch-elf"
	@echo "  make MRS_TOOLCHAIN_ROOT=/toolchain"
	@echo ""
	@echo "Build artifacts location: build/<PROJECT>/<BUILD_TYPE>/"

# Include dependency files if they exist
-include $(ALL_DEPS)

# Show toolchain information
toolchain-info:
	@echo "Toolchain Configuration:"
	@echo "========================"
	@echo "MRS_TOOLCHAIN_ROOT: $(MRS_TOOLCHAIN_ROOT)"
	@echo "TOOLCHAIN_PREFIX:   $(TOOLCHAIN_PREFIX)"
	@echo "TOOLCHAIN_BIN_DIR:  $(TOOLCHAIN_BIN_DIR)"
	@echo ""
	@echo "Active Tools:"
	@echo "CC:      $(CC)"
	@echo "OBJCOPY: $(OBJCOPY)"
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

# Phony targets
.PHONY: all build-single clean clean-all size memory-usage flash flash-bootloader erase debug help toolchain-info \
        check-toolchain all-release all-debug post-build combine-release
