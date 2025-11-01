@echo off
REM PFF Test Data Creator Script for Windows
REM This script creates test files on a USB drive for testing the PFF implementation

echo PFF Test Data Creator (Windows)
echo ==================================

REM Check if USB drive path is provided
if "%1"=="" (
    echo Usage: %0 ^<drive_letter^>
    echo Example: %0 D:
    goto :EOF
)

set USB_PATH=%1

REM Check if drive exists
if not exist "%USB_PATH%\" (
    echo Error: Drive %USB_PATH% does not exist
    goto :EOF
)

echo Creating test files on %USB_PATH%

REM Create README.TXT
echo PFF Test File - README.TXT > "%USB_PATH%\README.TXT"
echo ========================== >> "%USB_PATH%\README.TXT"
echo. >> "%USB_PATH%\README.TXT"
echo This file is created to test the Petit FatFS (PFF) implementation >> "%USB_PATH%\README.TXT"
echo on the CH32V203 USB host bootloader. >> "%USB_PATH%\README.TXT"
echo. >> "%USB_PATH%\README.TXT"
echo Test Information: >> "%USB_PATH%\README.TXT"
echo - File system: FAT32 (recommended) >> "%USB_PATH%\README.TXT"
echo - Sector size: 512 bytes >> "%USB_PATH%\README.TXT"
echo - Character encoding: ASCII >> "%USB_PATH%\README.TXT"
echo. >> "%USB_PATH%\README.TXT"
echo This file contains some test data that can be read by the PFF >> "%USB_PATH%\README.TXT"
echo library to verify proper USB Mass Storage functionality. >> "%USB_PATH%\README.TXT"
echo. >> "%USB_PATH%\README.TXT"
echo Test patterns: >> "%USB_PATH%\README.TXT"
echo 1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ >> "%USB_PATH%\README.TXT"
echo abcdefghijklmnopqrstuvwxyz0123456789 >> "%USB_PATH%\README.TXT"
echo. >> "%USB_PATH%\README.TXT"
echo End of test file. >> "%USB_PATH%\README.TXT"

REM Create TEST.TXT with more content
echo Extended PFF Test File > "%USB_PATH%\TEST.TXT"
echo ===================== >> "%USB_PATH%\TEST.TXT"
echo. >> "%USB_PATH%\TEST.TXT"
echo This is a larger test file to verify file reading capabilities >> "%USB_PATH%\TEST.TXT"
echo and seek operations in the PFF library. >> "%USB_PATH%\TEST.TXT"
echo. >> "%USB_PATH%\TEST.TXT"

REM Add more content for seek testing
for /l %%i in (1,1,50) do (
    echo Test line %%i - This line contains test data for seek operations >> "%USB_PATH%\TEST.TXT"
)

echo End of extended test file. >> "%USB_PATH%\TEST.TXT"

REM Create IAP test file
echo This is a test IAP update file. > "%USB_PATH%\RISKYKVM.UPD"
echo In a real bootloader, this would contain the firmware binary. >> "%USB_PATH%\RISKYKVM.UPD"
echo For testing purposes, this is just text content. >> "%USB_PATH%\RISKYKVM.UPD"
echo. >> "%USB_PATH%\RISKYKVM.UPD"
echo Test firmware data: >> "%USB_PATH%\RISKYKVM.UPD"

REM Add some content to make it larger
for /l %%i in (1,1,100) do (
    echo FIRMWARE_DATA_BLOCK_%%i_0123456789ABCDEF >> "%USB_PATH%\RISKYKVM.UPD"
)

REM Create a subdirectory
if not exist "%USB_PATH%\TESTDIR" mkdir "%USB_PATH%\TESTDIR"
echo Test file in subdirectory > "%USB_PATH%\TESTDIR\SUBTEST.TXT"

echo.
echo Test files created successfully!
echo.
echo Created files:
dir /B "%USB_PATH%\*.TXT" "%USB_PATH%\*.UPD" 2>nul
echo.
echo File descriptions:
echo - README.TXT: Basic text file for simple read tests
echo - TEST.TXT: Larger text file for seek and extended read tests  
echo - RISKYKVM.UPD: IAP update file (what the bootloader looks for)
echo - TESTDIR\: Subdirectory with test file
echo.
echo You can now insert the USB drive into the device and run tests.