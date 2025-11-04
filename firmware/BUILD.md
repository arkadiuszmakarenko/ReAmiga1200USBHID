# RISKYREA1200 Build Instructions

## Overview

This unified Makefile builds three project variants from different source directories:

1. **Bootloader** - Custom bootloader
   - FLASH: 0x00000000 - 0x00002FFF (12K)
   - Source: `Bootloader/`
   - Linker: `Bootloader/Ld/Link.ld`

2. **RISKYREA1200** - Standalone application
   - FLASH: 0x00000000 - 0x0000FFFF (64K)
   - Source: `src/`
   - Linker: `RISKYREA1200/Ld/Link.ld`

3. **RISKYREA1200Bootloader** - Application for custom bootloader
   - FLASH: 0x00003000 - 0x00010000 (54K, bootloader reserves first 12K)
   - Source: `src/`
   - Linker: `RISKYREA1200Boorloader/Ld/Link.ld`

## Quick Start

```bash
# Build standalone application (Release)
make

# Build bootloader (Debug)
make PROJECT=Bootloader BUILD_TYPE=Debug

# Build bootloader variant (Release)
make PROJECT=RISKYREA1200Bootloader

# Build standalone in Debug mode
make PROJECT=RISKYREA1200 BUILD_TYPE=Debug

# Build all three variants in Debug
make all-debug

# Build all three variants in Release
make all-release

# Build combined release binary (bootloader + app)
make release-combined

# Or combine manually if both are already built
make combine-release

# Clean all builds
make clean-all
```

**Note**: The `all-debug` and `all-release` targets automatically create `RISKYREA1200Release.bin` after building all three projects.

## Combined Release Binary

The `release-combined` target creates a single 64KB binary containing both the bootloader and application:

```bash
make release-combined
```

This produces `build/RISKYREA1200Release.bin` with the following layout:
- **0x00000000 - 0x00002FFF** (12KB): Bootloader
- **0x00003000 - 0x0000FFFF** (52KB): Application

The combined binary can be flashed directly to the device starting at address 0x00000000.

## Build Outputs

Build artifacts are organized by project and build type. All builds produce `.elf`, `.hex`, `.bin`, `.lst`, and `.map` files. The `RISKYREA1200Bootloader` variant also creates `RISKYREA.UPD` for bootloader updates.

The `release-combined` target creates an additional combined binary:

```
build/
├── RISKYREA1200Release.bin       ← Combined bootloader + app (64KB)
├── Bootloader/
│   ├── Debug/
│   │   ├── Bootloader.elf
│   │   ├── Bootloader.hex
│   │   ├── Bootloader.bin
│   │   ├── Bootloader.lst
│   │   └── Bootloader.map
│   └── Release/
│       └── ...
├── RISKYREA1200/
│   ├── Debug/
│   │   ├── RISKYREA1200.elf
│   │   ├── RISKYREA1200.hex
│   │   ├── RISKYREA1200.bin
│   │   ├── RISKYREA1200.lst
│   │   └── RISKYREA1200.map
│   └── Release/
│       └── ...
└── RISKYREA1200Bootloader/
    ├── Debug/
    │   ├── RISKYREA1200Bootloader.elf
    │   ├── RISKYREA1200Bootloader.hex
    │   ├── RISKYREA1200Bootloader.bin
    │   ├── RISKYREA.UPD              ← Bootloader update file
    │   └── ...
    └── Release/
        └── ...
```

## Build Types

### Debug
- Full debug symbols (`-g3 -ggdb3 -gdwarf-4`)
- Optimization: `-Og` (optimize for debugging)
- No inlining for better stepping
- Variable tracking enabled
- Larger binary, easier debugging

### Release
- Size optimization: `-Os`
- Link-time optimization (LTO)
- Debug symbols stripped
- Smaller binary, harder to debug

## Output File Formats

| Format | Description | Use Case |
|--------|-------------|----------|
| `.elf` | Executable with debug symbols | Debugging in GDB/VS Code |
| `.hex` | Intel HEX format | Flashing via OpenOCD/programmer |
| `.bin` | Raw binary | Direct flash programming |
| `.lst` | Disassembly listing | Code analysis |
| `.map` | Memory map | Linker analysis |
| `RISKYREA.UPD` | Update binary (bootloader variant only) | Bootloader firmware updates |
| `RISKYREA1200Release.bin` | Combined bootloader + app (64KB) | Full device programming |

## Makefile Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `PROJECT` | `RISKYREA1200` | Project variant to build |
| `BUILD_TYPE` | `Release` | Build configuration |
| `MRS_TOOLCHAIN_ROOT` | `../MRS_Toolchain_Linux_x64_V210` | Toolchain path |
| `TOOLCHAIN_VERSION` | `RISC-V Embedded GCC12` | GCC version |

## Examples

```bash
# Debug build for bootloader
make PROJECT=Bootloader BUILD_TYPE=Debug

# Debug build for bootloader variant
make PROJECT=RISKYREA1200Bootloader BUILD_TYPE=Debug

# Create combined release binary
make release-combined

# Flash standalone application
make PROJECT=RISKYREA1200 flash

# Flash combined binary
make flash-bin PROJECT=RISKYREA1200  # (manually specify combined binary)

# Show memory usage
make PROJECT=RISKYREA1200Bootloader memory-usage

# Build everything in Debug
make all-debug

# Custom toolchain path
make MRS_TOOLCHAIN_ROOT=/opt/toolchain PROJECT=RISKYREA1200

# Use system toolchain instead of MRS
make USE_SYSTEM_TOOLCHAIN=1
```

## VS Code Integration

The workspace has debug profiles for the Bootloader project. To add profiles for the main projects, update `.vscode/launch.json` to point to:
- `build/Bootloader/Debug/Bootloader.elf`
- `build/RISKYREA1200/Debug/RISKYREA1200.elf`
- `build/RISKYREA1200Bootloader/Debug/RISKYREA1200Bootloader.elf`

## Key Differences Between Variants

| Feature | Bootloader | RISKYREA1200 | RISKYREA1200Bootloader |
|---------|------------|--------------|------------------------|
| Flash Start | 0x00000000 | 0x00000000 | 0x00003000 |
| Flash Size | 12K | 64K | 54K |
| Source Dir | `Bootloader/` | `src/` | `src/` |
| Purpose | Boot and update | Standalone app | App with bootloader |
| Update Method | Flash programmer | Full flash | Via bootloader |

## Toolchain

The Makefile uses the bundled MRS RISC-V toolchain by default:
- GCC 12.2.0 (riscv-wch-elf)
- Located in: `../MRS_Toolchain_Linux_x64_V210/RISC-V Embedded GCC12/`

Run `make toolchain-info` to verify toolchain setup.
