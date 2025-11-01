# Expected Test Output Examples

## Quick Test Output (Normal Boot)

```
SystemClk:144000000
ChipID:12345678
Bootloader with PFF Library Test Suite
USB Host & UDisk Lib Initialization. 
USBFS Host Init
PFF library Initialization. 
Running PFF quick test...

=== PFF Quick Test ===
PASS: Disk initialization
PASS: Filesystem mount
PASS: Found IAP file (size: 2048 bytes)
=== Quick Test Complete ===

Starting normal IAP operation...
Enum:
Wait Disk Ready...
Disk Ready and Mounted Code:00.
PFF_DiskStatus:02
File Found, Start IAP Process
File size in bytes: 2048.
```

## Full Test Suite Output (PA0 Low)

```
SystemClk:144000000
ChipID:12345678
Bootloader with PFF Library Test Suite
USB Host & UDisk Lib Initialization. 
USBFS Host Init
PFF library Initialization. 
Running PFF quick test...

=== PFF Quick Test ===
PASS: Disk initialization
PASS: Filesystem mount
PASS: Found IAP file (size: 2048 bytes)
=== Quick Test Complete ===

Test mode detected (PA0 low) - Running full PFF test suite...

*****************************************************
         PFF (Petit FatFS) Test Suite v1.0          
     Testing USB Mass Storage Implementation         
*****************************************************

=== Test 1: Disk Initialization ===
PASS: Disk initialization successful

=== Test 2: Filesystem Mount ===
PASS: Filesystem mount (Expected: 0, Got: 0)
Filesystem type: FAT32

=== Test 3: Directory Listing ===
PASS: Open root directory (Expected: 0, Got: 0)
Root directory contents:
  F      512 README.TXT
  F     1856 TEST.TXT
  F     2048 RISKYKVM.UPD
  D        0 TESTDIR
Total files/directories found: 4
PASS: Directory listing completed

=== Test 4: File Open and Read ===
Successfully opened /RISKYKVM.UPD
File size: 2048 bytes
PASS: Read from IAP file (Expected: 0, Got: 0)
PASS: Read byte count valid
Read 256 bytes from IAP file
First 64 bytes (hex): 54 68 69 73 20 69 73 20 61 20 74 65 73 74 20 49 
                      41 50 20 75 70 64 61 74 65 20 66 69 6C 65 2E 0D 
                      0A 49 6E 20 61 20 72 65 61 6C 20 62 6F 6F 74 6C 
                      6F 61 64 65 72 2C 20 74 68 69 73 20 77 6F 75 6C 

=== Test 5: File Seek Test ===
Testing file seek operations
PASS: Seek to middle of file (Expected: 0, Got: 0)
Seeked to position 1024
PASS: Read after seek (Expected: 0, Got: 0)
Read 64 bytes from seek position
PASS: Seek near end of file (Expected: 0, Got: 0)
PASS: Read at end of file (Expected: 0, Got: 0)
Read 64 bytes near end of file

=== Test 6: File Write Test ===
Write functionality disabled in PFF configuration
PASS: Write test skipped (disabled)

=== Test 7: Stress Test ===
Iteration 1/5: OK (read 512 bytes)
Iteration 2/5: OK (read 512 bytes)
Iteration 3/5: OK (read 512 bytes)
Iteration 4/5: OK (read 512 bytes)
Iteration 5/5: OK (read 512 bytes)

==================================================
PFF Test Suite Summary
==================================================
Tests Passed: 18
Tests Failed: 0
Total Tests:  18
Result: ALL TESTS PASSED! ✓
==================================================

Tests completed. Remove test jumper and reset to run normal IAP.
```

## Failure Examples

### USB Device Not Found
```
=== Test 1: Disk Initialization ===
Disk initialization failed with status: 0x02
  - No medium in drive
FAIL: Disk initialization successful
```

### Filesystem Mount Failure
```
=== Test 2: Filesystem Mount ===
Mount failed with result: 6
  - No valid filesystem
FAIL: Filesystem mount (Expected: 0, Got: 6)
```

### File Not Found
```
=== Test 4: File Open and Read ===
IAP file not found, trying test file...
FAIL: Open any test file (no suitable file found)
```

### USB Communication Error
```
=== Test 4: File Open and Read ===
Successfully opened /RISKYKVM.UPD
File size: 2048 bytes
File read error: 1
FAIL: Read from IAP file (Expected: 0, Got: 1)
```

## LED Indicators

- **Success**: 3 short blinks (200ms each)
- **Failure**: 10 fast blinks (100ms each)  
- **Test Mode Active**: 1 blink every 2 seconds
- **Normal IAP**: Varies based on IAP operation

## Serial Monitor Settings

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None