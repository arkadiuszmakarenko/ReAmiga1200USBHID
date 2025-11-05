# Automated Build Process

This document describes the automated build process for the RISKYREA1200 firmware using GitHub Actions.

## Overview

The firmware is built automatically using a containerized environment based on Ubuntu 22.04. The workflow downloads and installs the WCH RISC-V toolchain, builds all firmware variants, and uploads the build artifacts for easy access.

## Workflow Triggers

The build workflow runs automatically on:
- Push to `main`, `master`, or `develop` branches
- Pull requests targeting `main`, `master`, or `develop` branches
- Manual trigger via the GitHub Actions UI (workflow_dispatch)

## Build Environment

- **Container**: Ubuntu 22.04
- **Toolchain**: WCH RISC-V toolchain V2.10 (Linux only, without IDE)
- **Toolchain Source**: https://github.com/arkadiuszmakarenko/WCHToolchain/releases/tag/ToolchainV210
- **Toolchain Path**: `/opt/riscv-toolchain/RISC-V Embedded GCC12/bin`
- **Build Tool**: GNU Make with parallel execution (`-j$(nproc)`)

## Build Process

The workflow performs the following steps:

1. **Repository Checkout**: Clones the repository code
2. **Install Dependencies**: Installs system build tools (build-essential, make, wget, xz-utils, tar)
3. **Download Toolchain**: Downloads WCH RISC-V toolchain V2.10 from GitHub release
4. **Verify Toolchain**: Checks toolchain installation and compiler execution
5. **Build Projects**: Builds all three firmware variants:
   - Bootloader (Debug build)
   - RISKYREA1200 (Release build)
   - RISKYREA1200Bootloader (Debug build)
6. **Create Combined Binary**: Combines bootloader and application into a single release binary
7. **Upload Artifacts**: Uploads all build outputs for download

## Build Artifacts

The following artifacts are generated and uploaded:

### Bootloader-Debug
- `Bootloader.bin` - Binary for bootloader (12KB at 0x00000000)
- `Bootloader.hex` - Intel HEX format
- `Bootloader.elf` - ELF executable with debug symbols

### RISKYREA1200-Release
- `RISKYREA1200.bin` - Standalone application binary (64KB at 0x00000000)
- `RISKYREA1200.hex` - Intel HEX format
- `RISKYREA1200.elf` - ELF executable

### RISKYREA1200Bootloader-Debug
- `RISKYREA1200Bootloader.bin` - Application binary for bootloader (52KB at 0x00003000)
- `RISKYREA1200Bootloader.hex` - Intel HEX format
- `RISKYREA1200Bootloader.elf` - ELF executable with debug symbols
- `RISKYREA.UPD` - Bootloader update file (copy of .bin for OTA updates)

### RISKYREA1200Release-Combined
- `RISKYREA1200Release.bin` - Combined bootloader + application (64KB total)
  - 0x00000000 - 0x00002FFF: Bootloader (12KB)
  - 0x00003000 - 0x0000FFFF: Application (52KB)

## Build Output Location

All build artifacts are stored in `firmware/build/` with the following structure:
```
firmware/build/
├── Bootloader/Debug/
│   ├── Bootloader.bin
│   ├── Bootloader.hex
│   ├── Bootloader.elf
│   ├── Bootloader.lst
│   └── Bootloader.map
├── RISKYREA1200/Release/
│   ├── RISKYREA1200.bin
│   ├── RISKYREA1200.hex
│   ├── RISKYREA1200.elf
│   ├── RISKYREA1200.lst
│   └── RISKYREA1200.map
├── RISKYREA1200Bootloader/Debug/
│   ├── RISKYREA1200Bootloader.bin
│   ├── RISKYREA1200Bootloader.hex
│   ├── RISKYREA1200Bootloader.elf
│   ├── RISKYREA1200Bootloader.lst
│   ├── RISKYREA1200Bootloader.map
│   └── RISKYREA.UPD
└── RISKYREA1200Release.bin
```

## Downloading Build Artifacts

1. Navigate to the GitHub Actions tab in the repository
2. Click on the latest successful workflow run
3. Scroll down to the "Artifacts" section
4. Download the desired artifact package
5. Extract the ZIP file to access the binaries

## Troubleshooting

### Toolchain Installation Issues

If the workflow fails during toolchain verification:
- Check that the toolchain URL is accessible
- Verify the toolchain directory structure matches expectations
- Ensure the compiler binary has execute permissions

The workflow includes diagnostic steps that will:
- List available directories if the toolchain path is not found
- Display available files if the compiler is missing
- Check compiler execution and display version information

### Build Failures

If a build step fails:
- Check the build logs for specific error messages
- Verify that all source files are present in the repository
- Ensure the Makefile configuration is correct for the toolchain path
- Confirm that the linker scripts are present and valid

### Makefile Configuration

The workflow uses the following Makefile parameters:
```bash
make PROJECT=<variant> BUILD_TYPE=<type> \
  MRS_TOOLCHAIN_ROOT=/opt/riscv-toolchain \
  -j$(nproc)
```

Where:
- `PROJECT`: Bootloader, RISKYREA1200, or RISKYREA1200Bootloader
- `BUILD_TYPE`: Release or Debug
- `MRS_TOOLCHAIN_ROOT`: Path to the extracted toolchain

## Local Build with Container

To replicate the CI build environment locally using Docker:

```bash
# Create a Dockerfile
cat > Dockerfile <<EOF
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    wget \
    xz-utils \
    tar

RUN wget -q \
  https://github.com/arkadiuszmakarenko/WCHToolchain/releases/download/ToolchainV210/MRS_Toolchain_Linux_x64_V210.tar.xz \
  -O /tmp/toolchain.tar.xz && \
  mkdir -p /opt/riscv-toolchain && \
  tar -xf /tmp/toolchain.tar.xz -C /opt/riscv-toolchain --strip-components=1 && \
  rm /tmp/toolchain.tar.xz

WORKDIR /workspace
EOF

# Build and run
docker build -t riskyrea1200-build .
docker run -v $(pwd):/workspace riskyrea1200-build bash -c "cd firmware && make all-debug MRS_TOOLCHAIN_ROOT=/opt/riscv-toolchain"
```

## Toolchain Customization

The Makefile supports several environment variables for toolchain configuration:

- `MRS_TOOLCHAIN_ROOT`: Root directory of the MounRiver toolchain
- `TOOLCHAIN_VERSION`: Toolchain version directory (default: "RISC-V Embedded GCC12")
- `TOOLCHAIN_PREFIX`: Tool prefix (default: "riscv-wch-elf")
- `USE_SYSTEM_TOOLCHAIN=1`: Use system-installed toolchain instead

Example with custom toolchain path:
```bash
make PROJECT=RISKYREA1200 BUILD_TYPE=Release \
  MRS_TOOLCHAIN_ROOT=/custom/path/to/toolchain \
  -j$(nproc)
```

## Workflow Status Badge

Add this badge to your README.md to show the build status:

```markdown
[![Build Firmware](https://github.com/arkadiuszmakarenko/RISKYREA1200/actions/workflows/build-firmware.yml/badge.svg)](https://github.com/arkadiuszmakarenko/RISKYREA1200/actions/workflows/build-firmware.yml)
```

## References

- [WCH RISC-V Toolchain V2.10](https://github.com/arkadiuszmakarenko/WCHToolchain/releases/tag/ToolchainV210)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [WCH CH32V RISC-V MCU Resources](https://github.com/openwch)
