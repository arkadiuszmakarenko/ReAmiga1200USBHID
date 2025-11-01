/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "usb_host_config.h"
#include "ch32v20x_usb.h"
#include "ch32v20x_usbfs_host.h"
#include "usb_host_iap.h"
#include "debug.h"
#include "string.h"

/* Local variables for USB Mass Storage */
static uint8_t msc_bulk_in_endp = 0;
static uint8_t msc_bulk_out_endp = 0;
static uint8_t msc_bulk_in_tog = 0;
static uint8_t msc_bulk_out_tog = 0;
static uint32_t msc_tag_counter = 0;
static uint8_t disk_initialized = 0;  // Track initialization state

/* Write operation state variables */
static uint8_t write_sector_buffer[512];  // Sector buffer for write operations
static DWORD current_write_sector = 0;   // Current sector being written
static UINT write_buffer_offset = 0;     // Current offset in write buffer
static uint8_t write_operation_active = 0; // Flag indicating write operation is active

/* SCSI command opcodes */
#define SCSI_READ_10        0x28
#define SCSI_WRITE_10       0x2A
#define SCSI_READ_CAPACITY  0x25
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_INQUIRY        0x12

/* USB Mass Storage Class specific definitions */
#define MSC_CLASS           0x08
#define MSC_SUBCLASS_SCSI   0x06
#define MSC_PROTOCOL_BOT    0x50
#define USB_ENDP_TYPE_BULK  0x02

/* Forward declarations */
static DRESULT msc_send_cbw(uint8_t *cdb, uint8_t cdb_len, uint32_t data_len, uint8_t direction);
static DRESULT msc_receive_csw(void);
static DRESULT msc_test_unit_ready(void);
static DRESULT msc_write_sector(DWORD sector, uint8_t* buffer);
static void reset_endpoints(void);

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
    DSTATUS stat = STA_NOINIT;
    uint8_t i, j;
    uint8_t usb_port = 0; // DEF_USB_PORT_FS = 0x00
    
    // DUG_PRINTF("=== Disk Initialize Debug ===\r\n");
    
    // Check if already initialized
    if (disk_initialized) {
        // DUG_PRINTF("Disk already initialized, skipping re-initialization\r\n");
        return 0; // Success
    }
    
    // Check if USB device is connected and enumerated
    if (IAP_Get_USB_Status(usb_port) != ROOT_DEV_SUCCESS) {
        DUG_PRINTF("USB device not ready, status: %d\r\n", IAP_Get_USB_Status(usb_port));
        return STA_NODISK;
    }
    
    // DUG_PRINTF("USB device ready, searching for MSC interface...\r\n");
    // DUG_PRINTF("Interface count: %d\r\n", HostCtl[usb_port].InterfaceNum);
    
    // Reset endpoint addresses
    msc_bulk_in_endp = 0;
    msc_bulk_out_endp = 0;
    
    // Find Mass Storage Class interface and bulk endpoints
    for (i = 0; i < HostCtl[usb_port].InterfaceNum; i++) {
        // DUG_PRINTF("Interface %d: Type=0x%02x, InEndp=%d, OutEndp=%d\r\n", 
        //            i, HostCtl[usb_port].Interface[i].Type,
        //            HostCtl[usb_port].Interface[i].InEndpNum,
        //            HostCtl[usb_port].Interface[i].OutEndpNum);
                   
        if (HostCtl[usb_port].Interface[i].Type == MSC_CLASS) {
            // DUG_PRINTF("Found MSC interface %d\r\n", i);
            
            // Find bulk IN endpoint
            for (j = 0; j < HostCtl[usb_port].Interface[i].InEndpNum; j++) {
                // DUG_PRINTF("IN EP %d: Addr=0x%02x, Type=0x%02x\r\n", 
                //            j, HostCtl[usb_port].Interface[i].InEndpAddr[j],
                //            HostCtl[usb_port].Interface[i].InEndpType[j]);
                if (HostCtl[usb_port].Interface[i].InEndpType[j] == USB_ENDP_TYPE_BULK) {
                    msc_bulk_in_endp = HostCtl[usb_port].Interface[i].InEndpAddr[j];
                    msc_bulk_in_tog = 0;
                    // DUG_PRINTF("Found Bulk IN endpoint: 0x%02x\r\n", msc_bulk_in_endp);
                    break;
                }
            }
            
            // Find bulk OUT endpoint  
            for (j = 0; j < HostCtl[usb_port].Interface[i].OutEndpNum; j++) {
                // DUG_PRINTF("OUT EP %d: Addr=0x%02x, Type=0x%02x\r\n", 
                //            j, HostCtl[usb_port].Interface[i].OutEndpAddr[j],
                //            HostCtl[usb_port].Interface[i].OutEndpType[j]);
                if (HostCtl[usb_port].Interface[i].OutEndpType[j] == USB_ENDP_TYPE_BULK) {
                    msc_bulk_out_endp = HostCtl[usb_port].Interface[i].OutEndpAddr[j];
                    msc_bulk_out_tog = 0;
                    // DUG_PRINTF("Found Bulk OUT endpoint: 0x%02x\r\n", msc_bulk_out_endp);
                    break;
                }
            }
            break;
        }
    }
    
    // Check if we found both endpoints
    if (msc_bulk_in_endp == 0 || msc_bulk_out_endp == 0) {
        DUG_PRINTF("Failed to find bulk endpoints: IN=0x%02x, OUT=0x%02x\r\n", 
                   msc_bulk_in_endp, msc_bulk_out_endp);
        return STA_NOINIT;
    }
    
    // DUG_PRINTF("Found endpoints: IN=0x%02x, OUT=0x%02x\r\n", msc_bulk_in_endp, msc_bulk_out_endp);
    
    // Test if device is ready
    // DUG_PRINTF("Testing unit ready...\r\n");
    
    // Add small delay to let device settle after endpoint discovery
    // Some USB devices need time between enumeration and first SCSI command
    for (volatile int i = 0; i < 20000; i++); // ~20ms delay (increased from 10ms)
    
    if (msc_test_unit_ready() == RES_OK) {
        // DUG_PRINTF("Unit ready - disk initialized successfully\r\n");
        
        // Additional settling time after Test Unit Ready
        for (volatile int i = 0; i < 10000; i++); // 10ms delay (increased from 5ms)
        
        disk_initialized = 1;  // Mark as initialized
        stat = 0; // Success
    } else {
        DUG_PRINTF("Unit not ready\r\n");
    }
    
    return stat;
}

/*-----------------------------------------------------------------------*/
/* Reset disk initialization state                                       */
/*-----------------------------------------------------------------------*/

void disk_reset_state(void)
{
    // DUG_PRINTF("Resetting disk state\r\n");
    disk_initialized = 0;
    msc_bulk_in_endp = 0;
    msc_bulk_out_endp = 0;
    msc_bulk_in_tog = 0;
    msc_bulk_out_tog = 0;
    msc_tag_counter = 0;
}

/*-----------------------------------------------------------------------*/
/* Helper function: Reset endpoints and clear any stall conditions      */
/*-----------------------------------------------------------------------*/
static void reset_endpoints(void)
{
    // Reset toggle bits
    msc_bulk_in_tog = 0;
    msc_bulk_out_tog = 0;
    
    // Add delay to let USB settle
    for (volatile int i = 0; i < 5000; i++); // 5ms delay
}

/*-----------------------------------------------------------------------*/
/* Helper function: Send SCSI command via USB Bulk-Only Transport       */
/*-----------------------------------------------------------------------*/
static DRESULT msc_send_cbw(uint8_t *cdb, uint8_t cdb_len, uint32_t data_len, uint8_t direction)
{
    UDISK_BOC_CBW cbw;
    uint8_t res;
    
    // Fill CBW structure
    cbw.mCBW_Sig = (USB_BO_CBW_SIG3 << 24) | (USB_BO_CBW_SIG2 << 16) | (USB_BO_CBW_SIG1 << 8) | USB_BO_CBW_SIG0;
    cbw.mCBW_Tag = ++msc_tag_counter;
    cbw.mCBW_DataLen = data_len;
    cbw.mCBW_Flag = direction; // 0x80 for IN, 0x00 for OUT
    cbw.mCBW_LUN = 0;
    cbw.mCBW_CB_Len = cdb_len;
    memcpy(cbw.mCBW_CB_Buf, cdb, cdb_len);
    memset(&cbw.mCBW_CB_Buf[cdb_len], 0, 16 - cdb_len);
    
    // DUG_PRINTF("Sending CBW: Tag=0x%08x, DataLen=%d, Dir=0x%02x, CDB[0]=0x%02x\r\n", 
    //            cbw.mCBW_Tag, cbw.mCBW_DataLen, cbw.mCBW_Flag, cdb[0]);
    
    // Send CBW
    res = USBFSH_SendEndpData(msc_bulk_out_endp, &msc_bulk_out_tog, (uint8_t*)&cbw, sizeof(cbw));
    if (res != ERR_SUCCESS) {
        DUG_PRINTF("CBW send failed: USB error %02x\r\n", res);
        return RES_ERROR;
    }
    
    // DUG_PRINTF("CBW sent successfully\r\n");
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Helper function: Receive CSW                                          */
/*-----------------------------------------------------------------------*/
static DRESULT msc_receive_csw(void)
{
    UDISK_BOC_CSW csw;
    uint16_t len;
    uint8_t res;
    
    // DUG_PRINTF("Receiving CSW...\r\n");
    
    // Receive CSW
    res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, (uint8_t*)&csw, &len);
    if (res != ERR_SUCCESS) {
        DUG_PRINTF("CSW receive failed: USB error %02x\r\n", res);
        
        // Try to recover from USB errors
        if (res == 0x2a || res == 0x2b) { // Handle both timeout and stall errors
            DUG_PRINTF("Attempting CSW error recovery...\r\n");
            
            // Comprehensive endpoint reset
            reset_endpoints();
            
            // Longer delay for device recovery
            for (volatile int i = 0; i < 10000; i++); // 10ms delay
            
            // Retry CSW receive
            res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, (uint8_t*)&csw, &len);
            if (res != ERR_SUCCESS) {
                DUG_PRINTF("CSW recovery failed: USB error %02x\r\n", res);
                return RES_ERROR;
            } else {
                DUG_PRINTF("CSW recovery successful\r\n");
            }
        } else {
            return RES_ERROR;
        }
    }
    
    if (len != sizeof(csw)) {
        DUG_PRINTF("CSW length error: expected %d, got %d\r\n", sizeof(csw), len);
        return RES_ERROR;
    }
    
    // DUG_PRINTF("CSW received: Sig=0x%08x, Tag=0x%08x, Residue=%d, Status=%d\r\n",
    //            csw.mCBW_Sig, csw.mCBW_Tag, csw.mCSW_Residue, csw.mCSW_Status);
    
    // Check CSW signature
    uint32_t expected_sig = ((USB_BO_CSW_SIG3 << 24) | (USB_BO_CSW_SIG2 << 16) | (USB_BO_CSW_SIG1 << 8) | USB_BO_CSW_SIG0);
    if (csw.mCBW_Sig != expected_sig) {
        DUG_PRINTF("CSW signature error: expected 0x%08x\r\n", expected_sig);
        return RES_ERROR;
    }
    
    // Check status
    if (csw.mCSW_Status != 0) {
        DUG_PRINTF("SCSI command failed with status %d\r\n", csw.mCSW_Status);
        return RES_ERROR;
    }
    
    // DUG_PRINTF("CSW OK\r\n");
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Helper function: Test Unit Ready command                              */
/*-----------------------------------------------------------------------*/
static DRESULT msc_test_unit_ready(void)
{
    uint8_t cdb[6] = {SCSI_TEST_UNIT_READY, 0, 0, 0, 0, 0};
    DRESULT cbw_res, csw_res;
    uint8_t retry_count = 0;
    
    // DUG_PRINTF("SCSI Test Unit Ready...\r\n");
    
    // Retry Test Unit Ready up to 3 times with delays
    do {
        cbw_res = msc_send_cbw(cdb, 6, 0, 0x00);
        if (cbw_res != RES_OK) {
            DUG_PRINTF("CBW send failed: %d\r\n", cbw_res);
            return RES_ERROR;
        }
        
        csw_res = msc_receive_csw();
        if (csw_res == RES_OK) {
            // DUG_PRINTF("Test Unit Ready: OK\r\n");
            return RES_OK;
        }
        
        // If failed, wait before retry
        if (retry_count < 2) {
            DUG_PRINTF("Test Unit Ready retry %d after delay...\r\n", retry_count + 1);
            for (volatile int i = 0; i < 10000; i++); // 10ms delay between retries
        }
        
        retry_count++;
    } while (retry_count < 3);
    
    DUG_PRINTF("Test Unit Ready failed after %d retries\r\n", retry_count);
    return csw_res;
}

/*-----------------------------------------------------------------------*/
/* Helper function: Write single sector using SCSI WRITE(10)            */
/*-----------------------------------------------------------------------*/
static DRESULT msc_write_sector(DWORD sector, uint8_t* buffer)
{
    uint8_t cdb[10];
    uint8_t usb_res;
    
    // Check if endpoints are initialized
    if (msc_bulk_in_endp == 0 || msc_bulk_out_endp == 0) {
        return RES_NOTRDY;
    }
    
    // Prepare WRITE(10) command
    cdb[0] = SCSI_WRITE_10;
    cdb[1] = 0;  // LUN = 0
    cdb[2] = (sector >> 24) & 0xFF;  // LBA MSB
    cdb[3] = (sector >> 16) & 0xFF;
    cdb[4] = (sector >> 8) & 0xFF;
    cdb[5] = sector & 0xFF;          // LBA LSB
    cdb[6] = 0;  // Reserved
    cdb[7] = 0;  // Transfer length MSB (writing 1 sector)
    cdb[8] = 1;  // Transfer length LSB
    cdb[9] = 0;  // Control
    
    // Send CBW for WRITE(10) command
    if (msc_send_cbw(cdb, 10, 512, 0x00) != RES_OK) {  // 0x00 = OUT direction
        return RES_ERROR;
    }
    
    // Send sector data in multiple USB packets (64 bytes each)
    uint16_t bytes_sent = 0;
    uint8_t packets_needed = 512 / 64;  // 8 packets of 64 bytes each
    
    for (uint8_t packet = 0; packet < packets_needed; packet++) {
        usb_res = USBFSH_SendEndpData(msc_bulk_out_endp, &msc_bulk_out_tog, 
                                      &buffer[bytes_sent], 64);
        if (usb_res != ERR_SUCCESS) {
            return RES_ERROR;
        }
        bytes_sent += 64;
    }
    
    // Receive CSW
    if (msc_receive_csw() != RES_OK) {
        return RES_ERROR;
    }
    
    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
	DRESULT res = RES_ERROR;
	uint8_t cdb[10];
	uint8_t sector_buffer[512];  // Standard sector size
	uint16_t len;
	uint8_t usb_res;

	// Check if endpoints are initialized
	if (msc_bulk_in_endp == 0 || msc_bulk_out_endp == 0) {
		return RES_NOTRDY;
	}

	// Prepare READ(10) command
	cdb[0] = SCSI_READ_10;
	cdb[1] = 0;  // LUN = 0
	cdb[2] = (sector >> 24) & 0xFF;  // LBA MSB
	cdb[3] = (sector >> 16) & 0xFF;
	cdb[4] = (sector >> 8) & 0xFF;
	cdb[5] = sector & 0xFF;          // LBA LSB
	cdb[6] = 0;  // Reserved
	cdb[7] = 0;  // Transfer length MSB (reading 1 sector)
	cdb[8] = 1;  // Transfer length LSB
	cdb[9] = 0;  // Control

	// Send CBW for READ(10) command
	// DUG_PRINTF("disk_readp: Reading sector %d, offset %d, count %d\r\n", (int)sector, offset, count);
	if (msc_send_cbw(cdb, 10, 512, 0x80) != RES_OK) {
		DUG_PRINTF("disk_readp: CBW send failed\r\n");
		return RES_ERROR;
	}

	// Read sector data
	// DUG_PRINTF("Reading sector data (512 bytes)...\r\n");
	
	// Read 512 bytes in multiple USB packets (64 bytes each)
	uint16_t bytes_read = 0;
	uint16_t packet_len;
	uint8_t packets_needed = 512 / 64;  // 8 packets of 64 bytes each
	
	for (uint8_t packet = 0; packet < packets_needed; packet++) {
		usb_res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, 
		                             &sector_buffer[bytes_read], &packet_len);
		if (usb_res != ERR_SUCCESS) {
			DUG_PRINTF("Data read failed at packet %d: USB error %02x\r\n", packet, usb_res);
			
			// Try comprehensive error recovery for USB errors
			if ((usb_res == 0x2a || usb_res == 0x2b) && packet == 0) {
				// First packet failed, device might need recovery
				DUG_PRINTF("Attempting comprehensive error recovery...\r\n");
				
				// Reset endpoints and wait longer
				reset_endpoints();
				for (volatile int i = 0; i < 20000; i++); // 20ms delay for recovery
				
				// Reset packet counter and try again from beginning
				bytes_read = 0;
				packet = 0;
				
				usb_res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, 
				                             &sector_buffer[bytes_read], &packet_len);
				if (usb_res != ERR_SUCCESS) {
					DUG_PRINTF("Recovery failed: USB error %02x\r\n", usb_res);
					return RES_ERROR;
				} else {
					DUG_PRINTF("Recovery successful, continuing...\r\n");
				}
			} else {
				return RES_ERROR;
			}
		}
		
		if (packet_len != 64) {
			DUG_PRINTF("Packet %d length error: expected 64, got %d\r\n", packet, packet_len);
			return RES_ERROR;
		}
		
		bytes_read += packet_len;
		// DUG_PRINTF("Packet %d: %d bytes (total: %d)\r\n", packet, packet_len, bytes_read);
	}
	
	if (bytes_read != 512) {
		DUG_PRINTF("Total data length error: expected 512, got %d\r\n", bytes_read);
		return RES_ERROR;
	}
	
	// DUG_PRINTF("Data read successful: %d bytes\r\n", bytes_read);
	// DUG_PRINTF("First 16 bytes: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
	//            sector_buffer[0], sector_buffer[1], sector_buffer[2], sector_buffer[3],
	//            sector_buffer[4], sector_buffer[5], sector_buffer[6], sector_buffer[7],
	//            sector_buffer[8], sector_buffer[9], sector_buffer[10], sector_buffer[11],
	//            sector_buffer[12], sector_buffer[13], sector_buffer[14], sector_buffer[15]);

	// Receive CSW
	if (msc_receive_csw() != RES_OK) {
		DUG_PRINTF("disk_readp: CSW receive failed\r\n");
		return RES_ERROR;
	}

	// Copy requested portion to destination buffer
	if (buff) {
		// Validate offset and count
		if (offset >= 512 || (offset + count) > 512) {
			return RES_PARERR;
		}
		memcpy(buff, &sector_buffer[offset], count);
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/

DRESULT disk_writep (
	BYTE* buff,		/* Pointer to the data to be written, NULL:Initiate/Finalize write operation */
	DWORD sc		/* Sector number (LBA) or Number of bytes to send */
)
{
	DRESULT res = RES_OK;

	if (!buff) {
		if (sc) {
			// Initiate write process - prepare for writing to specified sector
			current_write_sector = sc;
			write_buffer_offset = 0;
			write_operation_active = 1;
			
			// Clear the write buffer
			memset(write_sector_buffer, 0, sizeof(write_sector_buffer));
			
			// Note: In a complete implementation, we might want to read the existing
			// sector content first to preserve data that won't be overwritten
			// For simplicity, we're starting with a blank sector
		} else {
			// Finalize write process - flush any remaining data
			if (write_operation_active && write_buffer_offset > 0) {
				// Write the current sector buffer to disk
				res = msc_write_sector(current_write_sector, write_sector_buffer);
				if (res != RES_OK) {
					write_operation_active = 0;
					return res;
				}
			}
			
			// Reset write operation state
			write_operation_active = 0;
			write_buffer_offset = 0;
			current_write_sector = 0;
		}
	} else {
		// Send data to the disk buffer
		if (!write_operation_active) {
			return RES_ERROR; // Write operation not initiated
		}
		
		// sc contains the number of bytes to write
		UINT bytes_to_write = (UINT)sc;
		
		// Check if we have space in the current sector buffer
		if ((write_buffer_offset + bytes_to_write) > 512) {
			// Data exceeds current sector, write current sector and continue
			res = msc_write_sector(current_write_sector, write_sector_buffer);
			if (res != RES_OK) {
				write_operation_active = 0;
				return res;
			}
			
			// Move to next sector and clear buffer
			current_write_sector++;
			write_buffer_offset = 0;
			memset(write_sector_buffer, 0, sizeof(write_sector_buffer));
			
			// Recursively handle remaining data if it still doesn't fit
			if (bytes_to_write > 512) {
				// Write full sectors directly
				while (bytes_to_write >= 512) {
					res = msc_write_sector(current_write_sector, buff);
					if (res != RES_OK) {
						write_operation_active = 0;
						return res;
					}
					current_write_sector++;
					buff += 512;
					bytes_to_write -= 512;
				}
			}
		}
		
		// Copy remaining data to sector buffer
		if (bytes_to_write > 0) {
			memcpy(&write_sector_buffer[write_buffer_offset], buff, bytes_to_write);
			write_buffer_offset += bytes_to_write;
		}
	}

	return res;
}

