# USB Enumeration Timing Fix

## Issue
The PFF test suite was failing because tests were running before USB device enumeration completed, resulting in "Drive not initialized" errors.

## Root Cause
The test functions were called immediately after `IAP_Initialization()` without waiting for the USB enumeration process to complete. The `IAP_USBH_PreDeal()` function needs to run multiple cycles to:
1. Detect USB device connection
2. Perform USB enumeration  
3. Discover endpoints
4. Set device status to `ROOT_DEV_SUCCESS`

## Solution
1. **Added helper function** `IAP_Get_USB_Status()` in `usb_host_iap.c`:
   - Safely returns USB device status for a given port
   - Avoids direct access to `RootHubDev[]` array from main.c

2. **Modified main.c enumeration sequence**:
   - Added enumeration wait loop with up to 50 attempts (5 seconds)
   - Calls `IAP_USBH_PreDeal()` repeatedly to advance enumeration
   - Monitors device status until `ROOT_DEV_SUCCESS` is achieved
   - Provides progress feedback every 10 attempts
   - Gracefully handles enumeration failures

3. **Improved error handling**:
   - Shows detailed enumeration progress
   - Runs tests anyway on failure to demonstrate error modes
   - Provides visual feedback via LED blinking patterns

## Code Changes

### usb_host_iap.h
```c
extern uint8_t IAP_Get_USB_Status (uint8_t port);
```

### usb_host_iap.c
```c
uint8_t IAP_Get_USB_Status (uint8_t port)
{
    if (port >= DEF_TOTAL_ROOT_HUB) {
        return 0;  // Invalid port
    }
    return RootHubDev[port].bStatus;
}
```

### main.c
- Added enumeration wait loop before running tests
- Uses `IAP_Get_USB_Status(0)` to check device readiness
- Provides detailed enumeration progress feedback
- Ensures USB device is ready before PFF operations

## Result
- Tests now run only after successful USB enumeration
- Better error reporting and debugging capabilities
- Graceful handling of USB connection issues
- Maintained backward compatibility with normal IAP operation

## USB Device Status Values
- `ROOT_DEV_SUCCESS` (3): Device ready for operations
- `ROOT_DEV_CONNECTED` (2): Device connected but not enumerated
- `ROOT_DEV_FAILED` (4): Enumeration failed
- `0`: No device or uninitialized

## Testing
The enumeration fix ensures that:
1. USB Mass Storage device is properly detected
2. Endpoints are discovered and configured
3. SCSI command interface is ready
4. PFF library can successfully mount and access the filesystem

This resolves the timing issue where tests would fail with "Drive not initialized" status 0x01.