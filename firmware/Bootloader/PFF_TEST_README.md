# PFF Library Test Suite

This test suite verifies the Petit FatFS (PFF) implementation for USB Mass Storage devices on the CH32V203 bootloader.

## Overview

The test suite includes comprehensive tests for:
- USB Mass Storage device initialization
- Filesystem mounting (FAT12/16/32)
- Directory listing operations
- File open and read operations
- File seek operations  
- File write operations (if enabled)
- Stress testing with multiple operations

## Files

- `pff_test.c` - Main test suite implementation
- `pff_test.h` - Test suite header file
- `create_test_files.sh` - Linux/Mac script to create test files on USB drive
- `create_test_files.bat` - Windows script to create test files on USB drive

## Test Modes

### Quick Test Mode (Default)
- Runs automatically on every boot
- Tests basic disk initialization and filesystem mounting
- Checks for IAP file presence
- Minimal output for normal operation

### Full Test Mode  
- Triggered by pulling PA0 pin low during boot
- Runs comprehensive test suite with detailed output
- Tests all PFF functionality
- Displays pass/fail results for each test

## Hardware Setup

1. **Normal Operation**: No special setup needed
2. **Full Test Mode**: Connect PA0 pin to GND before powering on

## USB Drive Preparation

### Option 1: Use Creation Scripts

**Linux/Mac:**
```bash
./create_test_files.sh /path/to/usb/drive
# Example: ./create_test_files.sh /media/usb
```

**Windows:**
```cmd
create_test_files.bat D:
# Replace D: with your USB drive letter
```

### Option 2: Manual File Creation

Create these files on your USB drive:

1. **README.TXT** - Basic text file for simple read tests
2. **TEST.TXT** - Larger text file (>1KB) for seek operations  
3. **RISKYKVM.UPD** - IAP update file (what bootloader looks for)
4. **TESTDIR/** - Subdirectory with test files

## Test Descriptions

### Test 1: Disk Initialization
- Verifies USB mass storage device detection
- Tests endpoint discovery and configuration
- Validates SCSI TEST UNIT READY command

### Test 2: Filesystem Mount
- Tests PFF filesystem mounting
- Verifies FAT filesystem recognition
- Displays filesystem type (FAT12/16/32)

### Test 3: Directory Listing
- Lists files in root directory
- Tests pf_opendir() and pf_readdir() functions
- Displays file names, sizes, and attributes

### Test 4: File Open and Read
- Tests pf_open() with different file types
- Reads file content and displays preview
- Validates read byte counts

### Test 5: File Seek Operations
- Tests pf_lseek() function
- Seeks to middle and end of files
- Verifies read after seek operations

### Test 6: File Write Operations
- Tests pf_write() function (if enabled in config)
- Creates test file with sample data
- Verifies write byte counts

### Test 7: Stress Test
- Performs multiple mount/read cycles
- Tests reliability under repeated operations
- Validates consistency across iterations

## Expected Output

### Successful Quick Test:
```
=== PFF Quick Test ===
PASS: Disk initialization
PASS: Filesystem mount
PASS: Found IAP file (size: 1234 bytes)
=== Quick Test Complete ===
```

### Successful Full Test:
```
*****************************************************
         PFF (Petit FatFS) Test Suite v1.0          
     Testing USB Mass Storage Implementation         
*****************************************************

=== Test 1: Disk Initialization ===
PASS: Disk initialization successful

=== Test 2: Filesystem Mount ===
PASS: Filesystem mount (Expected: 0, Got: 0)
Filesystem type: FAT32

[... more test output ...]

==================================================
PFF Test Suite Summary
==================================================
Tests Passed: 15
Tests Failed: 0
Total Tests:  15
Result: ALL TESTS PASSED! ✓
==================================================
```

## Troubleshooting

### Common Issues:

**"FAIL: Disk initialization"**
- Check USB device connection
- Verify USB host enumeration completed
- Ensure device is mass storage class

**"FAIL: Filesystem mount"**  
- Check USB drive formatting (use FAT32)
- Verify drive is not corrupted
- Try different USB drive

**"FAIL: Open any test file"**
- Run file creation scripts first
- Check file names match exactly
- Verify files are in root directory

**"No suitable file for seek test"**
- Ensure TEST.TXT is larger than 1KB
- Recreate test files with scripts

### Debug Output

Monitor serial output at 115200 baud to see detailed test results and debug information.

## PFF Configuration

The test suite requires these PFF features enabled in `pffconf.h`:

```c
#define PF_USE_READ     1   // Required for all tests
#define PF_USE_DIR      1   // Required for directory listing
#define PF_USE_LSEEK    1   // Required for seek tests  
#define PF_USE_WRITE    1   // Required for write tests
#define PF_FS_FAT32     1   // Recommended for compatibility
```

## Integration

To integrate the test suite into your project:

1. Add `pff_test.c` and `pff_test.h` to your project
2. Include `pff_test.h` in your main.c
3. Call `run_pff_quick_test()` for basic verification
4. Call `run_pff_test_suite()` for comprehensive testing

The test suite is designed to be non-intrusive and can remain in production code since the full test mode requires hardware jumper activation.