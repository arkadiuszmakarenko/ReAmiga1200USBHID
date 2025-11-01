#!/bin/bash

# PFF Test Data Creator Script
# This script creates test files on a USB drive for testing the PFF implementation

echo "PFF Test Data Creator"
echo "===================="

# Check if USB drive path is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <usb_drive_path>"
    echo "Example: $0 /media/usb or D:\\ (Windows)"
    exit 1
fi

USB_PATH="$1"

# Check if path exists
if [ ! -d "$USB_PATH" ]; then
    echo "Error: Path $USB_PATH does not exist"
    exit 1
fi

echo "Creating test files on $USB_PATH"

# Create README.TXT
cat > "$USB_PATH/README.TXT" << EOF
PFF Test File - README.TXT
==========================

This file is created to test the Petit FatFS (PFF) implementation
on the CH32V203 USB host bootloader.

Test Information:
- File system: FAT32 (recommended)
- Sector size: 512 bytes
- Character encoding: ASCII

This file contains some test data that can be read by the PFF
library to verify proper USB Mass Storage functionality.

Test patterns:
1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ
abcdefghijklmnopqrstuvwxyz0123456789

End of test file.
EOF

# Create TEST.TXT with more content
cat > "$USB_PATH/TEST.TXT" << EOF
Extended PFF Test File
=====================

This is a larger test file to verify file reading capabilities
and seek operations in the PFF library.

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod 
tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim 
veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea 
commodo consequat.

Duis aute irure dolor in reprehenderit in voluptate velit esse cillum 
dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non 
proident, sunt in culpa qui officia deserunt mollit anim id est laborum.

Test Data Section 1:
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

Test Data Section 2:
1111111111111111111111111111111111111111111111111111111111111111
2222222222222222222222222222222222222222222222222222222222222222
3333333333333333333333333333333333333333333333333333333333333333

This should provide enough content for seek testing.
The file should be over 1KB to properly test seek operations.

Test Data Section 3:
0000000000000000000000000000000000000000000000000000000000000000
1111111111111111111111111111111111111111111111111111111111111111
2222222222222222222222222222222222222222222222222222222222222222
3333333333333333333333333333333333333333333333333333333333333333
4444444444444444444444444444444444444444444444444444444444444444
5555555555555555555555555555555555555555555555555555555555555555

End of extended test file.
EOF

# Create a small binary test file (simulated firmware)
echo "Creating binary test file..."
dd if=/dev/zero of="$USB_PATH/FIRMWARE.BIN" bs=1024 count=4 2>/dev/null
# Add some pattern to the binary file
printf '\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F' > temp_pattern
for i in {1..256}; do
    cat temp_pattern >> "$USB_PATH/FIRMWARE.BIN"
done
rm temp_pattern

# Create the IAP file with a simple pattern
echo "Creating IAP test file..."
cat > "$USB_PATH/RISKYKVM.UPD" << EOF
This is a test IAP update file.
In a real bootloader, this would contain the firmware binary.
For testing purposes, this is just text content.

Test firmware data:
EOF

# Add some binary-like content
for i in {1..100}; do
    printf "FIRMWARE_DATA_BLOCK_%03d_" $i >> "$USB_PATH/RISKYKVM.UPD"
    printf '\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F' >> "$USB_PATH/RISKYKVM.UPD"
done

# Create a directory for testing
mkdir -p "$USB_PATH/TESTDIR"
echo "Test file in subdirectory" > "$USB_PATH/TESTDIR/SUBTEST.TXT"

# Show created files
echo ""
echo "Created test files:"
ls -la "$USB_PATH"/ | grep -E '\.(TXT|UPD|BIN)$|TESTDIR'

echo ""
echo "Test files created successfully!"
echo ""
echo "File descriptions:"
echo "- README.TXT: Basic text file for simple read tests"
echo "- TEST.TXT: Larger text file for seek and extended read tests"
echo "- FIRMWARE.BIN: Binary file with pattern data"
echo "- RISKYKVM.UPD: IAP update file (what the bootloader looks for)"
echo "- TESTDIR/: Subdirectory with test file"
echo ""
echo "You can now insert the USB drive into the device and run tests."