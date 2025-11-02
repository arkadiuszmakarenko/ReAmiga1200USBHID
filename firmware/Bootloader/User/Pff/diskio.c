/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for Petit FatFs (C)ChaN, 2014              */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "usb_host_config.h"
#include "ch32v20x_usb.h"
#include "ch32v20x_usbfs_host.h"
#include "usb_host_iap.h"
#include "debug.h"
#include "string.h"

/* USB Mass Storage variables */
static uint8_t msc_bulk_in_endp = 0;
static uint8_t msc_bulk_out_endp = 0;
static uint8_t msc_bulk_in_tog = 0;
static uint8_t msc_bulk_out_tog = 0;
static uint32_t msc_tag_counter = 0;
static uint8_t disk_initialized = 0;
static uint32_t last_lba = 0;
static uint32_t block_size = 0;

/* Write operation variables */
static uint8_t write_sector_buffer[512];
static DWORD current_write_sector = 0;
static UINT write_buffer_offset = 0;
static uint8_t write_operation_active = 0;

/* SCSI command opcodes */
#define SCSI_READ_10        0x28
#define SCSI_WRITE_10       0x2A
#define SCSI_READ_CAPACITY  0x25
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_INQUIRY        0x12

/* USB Mass Storage Class definitions */
#define MSC_CLASS           0x08
#define MSC_SUBCLASS_SCSI   0x06
#define MSC_PROTOCOL_BOT    0x50
#define USB_ENDP_TYPE_BULK  0x02

/* Forward declarations */
static DRESULT msc_send_cbw(uint8_t *cdb, uint8_t cdb_len, uint32_t data_len, uint8_t direction);
static DRESULT msc_receive_csw(void);
static DRESULT msc_test_unit_ready(void);
static DRESULT msc_write_sector(DWORD sector, uint8_t* buffer);
static DRESULT msc_mass_storage_reset(void);
static DRESULT msc_request_sense(uint8_t* sense_data);
static DRESULT msc_read_capacity(uint32_t* last_lba, uint32_t* block_size);

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(void)
{
    DSTATUS stat = STA_NOINIT;
    uint8_t i, j;
    uint8_t usb_port = 0;
    
    if (disk_initialized) {
        return 0;
    }
    
    if (IAP_Get_USB_Status(usb_port) != ROOT_DEV_SUCCESS) {
        return STA_NODISK;
    }
    
    msc_bulk_in_endp = 0;
    msc_bulk_out_endp = 0;
    
    /* Find Mass Storage Class interface and bulk endpoints */
    for (i = 0; i < HostCtl[usb_port].InterfaceNum; i++) {
        if (HostCtl[usb_port].Interface[i].Type == MSC_CLASS) {
            /* Find bulk IN endpoint */
            for (j = 0; j < HostCtl[usb_port].Interface[i].InEndpNum; j++) {
                if (HostCtl[usb_port].Interface[i].InEndpType[j] == USB_ENDP_TYPE_BULK) {
                    msc_bulk_in_endp = HostCtl[usb_port].Interface[i].InEndpAddr[j];
                    msc_bulk_in_tog = 0;
                    break;
                }
            }
            
            /* Find bulk OUT endpoint */
            for (j = 0; j < HostCtl[usb_port].Interface[i].OutEndpNum; j++) {
                if (HostCtl[usb_port].Interface[i].OutEndpType[j] == USB_ENDP_TYPE_BULK) {
                    msc_bulk_out_endp = HostCtl[usb_port].Interface[i].OutEndpAddr[j];
                    msc_bulk_out_tog = 0;
                    break;
                }
            }
            break;
        }
    }
    
    if (msc_bulk_in_endp == 0 || msc_bulk_out_endp == 0) {
        return STA_NOINIT;
    }
    
    /* Device stabilization delay */
    for (volatile int i = 0; i < 500000; i++);
    
    if (msc_test_unit_ready() == RES_OK) {
        for (volatile int i = 0; i < 50000; i++);
        
        if (msc_read_capacity(&last_lba, &block_size) == RES_OK) {
            for (volatile int i = 0; i < 50000; i++);
            disk_initialized = 1;
            stat = 0;
        }
    }
    
    return stat;
}

/*-----------------------------------------------------------------------*/
/* Reset disk initialization state                                       */
/*-----------------------------------------------------------------------*/
void disk_reset_state(void)
{
    disk_initialized = 0;
    msc_bulk_in_endp = 0;
    msc_bulk_out_endp = 0;
    msc_bulk_in_tog = 0;
    msc_bulk_out_tog = 0;
    msc_tag_counter = 0;
    last_lba = 0;
    block_size = 0;
}

/*-----------------------------------------------------------------------*/
/* Send SCSI command via USB Bulk-Only Transport                        */
/*-----------------------------------------------------------------------*/
static DRESULT msc_send_cbw(uint8_t *cdb, uint8_t cdb_len, uint32_t data_len, uint8_t direction)
{
    UDISK_BOC_CBW cbw;
    uint8_t res;
    int retry_count = 0;
    
    cbw.mCBW_Sig = (USB_BO_CBW_SIG3 << 24) | (USB_BO_CBW_SIG2 << 16) | 
                   (USB_BO_CBW_SIG1 << 8) | USB_BO_CBW_SIG0;
    cbw.mCBW_Tag = ++msc_tag_counter;
    cbw.mCBW_DataLen = data_len;
    cbw.mCBW_Flag = direction;
    cbw.mCBW_LUN = 0;
    cbw.mCBW_CB_Len = cdb_len;
    memcpy(cbw.mCBW_CB_Buf, cdb, cdb_len);
    memset(&cbw.mCBW_CB_Buf[cdb_len], 0, 16 - cdb_len);
    
    do {
        res = USBFSH_SendEndpData(msc_bulk_out_endp, &msc_bulk_out_tog, 
                                  (uint8_t*)&cbw, sizeof(cbw));
        if (res == ERR_SUCCESS) {
            return RES_OK;
        }
        
        if ((retry_count % 10) == 9) {
            uint8_t ep0 = 64;
            USBFSH_ClearEndpStall(ep0, msc_bulk_out_endp);
            msc_bulk_out_tog = 0;
            for (volatile int i = 0; i < 2000; i++);
        }
        for (volatile int i = 0; i < 1000; i++);
        retry_count++;
    } while (retry_count < 40);
    
    return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Receive CSW                                                           */
/*-----------------------------------------------------------------------*/
static DRESULT msc_receive_csw(void)
{
    UDISK_BOC_CSW csw;
    uint16_t len;
    uint8_t res;
    int retry_count = 0;
    
    do {
        res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, 
                                 (uint8_t*)&csw, &len);
        if (res == ERR_SUCCESS) {
            break;
        }
        
        if ((retry_count % 10) == 9) {
            uint8_t ep0 = 64;
            USBFSH_ClearEndpStall(ep0, (uint8_t)(0x80 | msc_bulk_in_endp));
            msc_bulk_in_tog = 0;
            for (volatile int i = 0; i < 2000; i++);
        }
        for (volatile int i = 0; i < 1000; i++);
        retry_count++;
    } while (retry_count < 60);
    
    if (res != ERR_SUCCESS || len != sizeof(csw)) {
        return RES_ERROR;
    }
    
    uint32_t expected_sig = ((USB_BO_CSW_SIG3 << 24) | (USB_BO_CSW_SIG2 << 16) | 
                            (USB_BO_CSW_SIG1 << 8) | USB_BO_CSW_SIG0);
    if (csw.mCBW_Sig != expected_sig || csw.mCSW_Status != 0) {
        return RES_ERROR;
    }
    
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Test Unit Ready command                                               */
/*-----------------------------------------------------------------------*/
static DRESULT msc_test_unit_ready(void)
{
    uint8_t cdb[6] = {SCSI_TEST_UNIT_READY, 0, 0, 0, 0, 0};
    DRESULT cbw_res, csw_res;
    uint8_t sense_data[18];
    
    for (int attempt = 0; attempt < 5; attempt++) {
        cbw_res = msc_send_cbw(cdb, 6, 0, 0x00);
        if (cbw_res != RES_OK) {
            return RES_ERROR;
        }
        
        csw_res = msc_receive_csw();
        if (csw_res == RES_OK) {
            return RES_OK;
        }
        
        if (msc_request_sense(sense_data) == RES_OK) {
            uint8_t sense_key = sense_data[2] & 0x0F;
            uint8_t asc = sense_data[12];
            uint8_t ascq = sense_data[13];
            
            if (sense_key == 0x02 && asc == 0x04 && ascq == 0x01) {
                for (volatile int i = 0; i < 200000; i++);
                continue;
            } else if (sense_key == 0x06) {
                for (volatile int i = 0; i < 50000; i++);
                continue;
            }
        }
        
        if (attempt < 2) {
            msc_mass_storage_reset();
            for (volatile int i = 0; i < 100000; i++);
        } else {
            for (volatile int i = 0; i < 100000; i++);
        }
    }
    
    return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Mass Storage Reset for error recovery                                 */
/*-----------------------------------------------------------------------*/
static DRESULT msc_mass_storage_reset(void)
{
    uint8_t ep0 = 64;
    
    USBFSH_ClearEndpStall(ep0, (uint8_t)(0x80 | msc_bulk_in_endp));
    for (volatile int i = 0; i < 2000; i++);
    
    USBFSH_ClearEndpStall(ep0, msc_bulk_out_endp);
    for (volatile int i = 0; i < 2000; i++);
    
    msc_bulk_in_tog = 0;
    msc_bulk_out_tog = 0;
    
    for (volatile int i = 0; i < 20000; i++);
    
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* REQUEST SENSE command                                                 */
/*-----------------------------------------------------------------------*/
static DRESULT msc_request_sense(uint8_t* sense_data)
{
    uint8_t cdb[6] = {0x03, 0, 0, 0, 18, 0};
    DRESULT cbw_res, csw_res;
    uint16_t len;
    uint8_t usb_res;
    int retry_count = 0;
    
    cbw_res = msc_send_cbw(cdb, 6, 18, 0x80);
    if (cbw_res != RES_OK) {
        return RES_ERROR;
    }
    
    for (volatile int i = 0; i < 1000; i++);
    
    do {
        len = 18;
        usb_res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, 
                                     sense_data, &len);
        if (usb_res == ERR_SUCCESS) {
            break;
        }
        
        if ((retry_count % 10) == 9) {
            uint8_t ep0 = 64;
            USBFSH_ClearEndpStall(ep0, (uint8_t)(0x80 | msc_bulk_in_endp));
            msc_bulk_in_tog = 0;
            for (volatile int i = 0; i < 2000; i++);
        }
        
        for (volatile int i = 0; i < 1000; i++);
        retry_count++;
    } while (retry_count < 40);
    
    if (usb_res != ERR_SUCCESS) {
        return RES_ERROR;
    }
    
    csw_res = msc_receive_csw();
    if (csw_res != RES_OK) {
        return RES_ERROR;
    }
    
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* READ CAPACITY command                                                 */
/*-----------------------------------------------------------------------*/
static DRESULT msc_read_capacity(uint32_t* last_lba, uint32_t* block_size)
{
    uint8_t cdb[10] = {SCSI_READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t capacity_data[8];
    DRESULT cbw_res, csw_res;
    uint16_t len;
    uint8_t usb_res;
    int retry_count = 0;
    
    cbw_res = msc_send_cbw(cdb, 10, 8, 0x80);
    if (cbw_res != RES_OK) {
        return RES_ERROR;
    }
    
    for (volatile int i = 0; i < 2000; i++);
    
    do {
        len = 8;
        usb_res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, 
                                     capacity_data, &len);
        if (usb_res == ERR_SUCCESS) {
            break;
        }
        
        if ((retry_count % 10) == 9) {
            uint8_t ep0 = 64;
            USBFSH_ClearEndpStall(ep0, (uint8_t)(0x80 | msc_bulk_in_endp));
            msc_bulk_in_tog = 0;
            for (volatile int i = 0; i < 2000; i++);
        }
        
        for (volatile int i = 0; i < 1000; i++);
        retry_count++;
    } while (retry_count < 40);
    
    if (usb_res != ERR_SUCCESS || len != 8) {
        return RES_ERROR;
    }
    
    csw_res = msc_receive_csw();
    if (csw_res != RES_OK) {
        return RES_ERROR;
    }
    
    *last_lba = (capacity_data[0] << 24) | (capacity_data[1] << 16) | 
                (capacity_data[2] << 8) | capacity_data[3];
    *block_size = (capacity_data[4] << 24) | (capacity_data[5] << 16) | 
                  (capacity_data[6] << 8) | capacity_data[7];
    
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write single sector using SCSI WRITE(10)                             */
/*-----------------------------------------------------------------------*/
static DRESULT msc_write_sector(DWORD sector, uint8_t* buffer)
{
    uint8_t cdb[10];
    uint8_t usb_res;
    
    if (msc_bulk_in_endp == 0 || msc_bulk_out_endp == 0) {
        return RES_NOTRDY;
    }
    
    cdb[0] = SCSI_WRITE_10;
    cdb[1] = 0;
    cdb[2] = (sector >> 24) & 0xFF;
    cdb[3] = (sector >> 16) & 0xFF;
    cdb[4] = (sector >> 8) & 0xFF;
    cdb[5] = sector & 0xFF;
    cdb[6] = 0;
    cdb[7] = 0;
    cdb[8] = 1;
    cdb[9] = 0;
    
    if (msc_send_cbw(cdb, 10, 512, 0x00) != RES_OK) {
        return RES_ERROR;
    }
    
    uint16_t bytes_sent = 0;
    uint8_t packets_needed = 512 / 64;
    
    for (uint8_t packet = 0; packet < packets_needed; packet++) {
        usb_res = USBFSH_SendEndpData(msc_bulk_out_endp, &msc_bulk_out_tog, 
                                      &buffer[bytes_sent], 64);
        if (usb_res != ERR_SUCCESS) {
            return RES_ERROR;
        }
        bytes_sent += 64;
    }
    
    if (msc_receive_csw() != RES_OK) {
        return RES_ERROR;
    }
    
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/
DRESULT disk_readp(BYTE* buff, DWORD sector, UINT offset, UINT count)
{
    uint8_t cdb[10];
    uint8_t sector_buffer[512];
    uint16_t packet_len;
    uint8_t usb_res;
    uint8_t sense_data[18];
    // More robust retry strategy to handle devices that hiccup at cluster boundaries
    for (int read_attempt = 0; read_attempt < 4; read_attempt++) {
        if (msc_bulk_in_endp == 0 || msc_bulk_out_endp == 0) {
            return RES_NOTRDY;
        }

        cdb[0] = SCSI_READ_10;
        cdb[1] = 0;
        cdb[2] = (sector >> 24) & 0xFF;
        cdb[3] = (sector >> 16) & 0xFF;
        cdb[4] = (sector >> 8) & 0xFF;
        cdb[5] = sector & 0xFF;
        cdb[6] = 0;
        cdb[7] = 0;
        cdb[8] = 1;
        cdb[9] = 0;

        if (msc_send_cbw(cdb, 10, 512, 0x80) != RES_OK) {
            goto read_retry;
        }

        // Small settle time before IN data stage
        for (volatile int i = 0; i < 3000; i++);

        uint16_t bytes_read = 0;
        uint8_t packets_needed = 512 / 64;
        uint8_t read_success = 1;
        
        for (uint8_t packet = 0; packet < packets_needed; packet++) {
            int nak_retries = 0;
            
            do {
                packet_len = 64;
                usb_res = USBFSH_GetEndpData(msc_bulk_in_endp, &msc_bulk_in_tog, 
                                             &sector_buffer[bytes_read], &packet_len);
                
                if (usb_res == ERR_SUCCESS) {
                    break;
                }
                
                // Periodically clear potential STALL and re-sync toggles
                if ((nak_retries % 10) == 9) {
                    uint8_t ep0 = 64;
                    USBFSH_ClearEndpStall(ep0, (uint8_t)(0x80 | msc_bulk_in_endp));
                    msc_bulk_in_tog = 0;
                    for (volatile int i = 0; i < 3000; i++);
                }
                
                // Backoff a touch before retrying
                for (volatile int i = 0; i < 1500; i++);
                nak_retries++;
                
            } while (nak_retries < 40 && usb_res != ERR_SUCCESS);
            
            if (usb_res != ERR_SUCCESS || packet_len != 64) {
                read_success = 0;
                break;
            }
            
            bytes_read += packet_len;
        }
        
        if (!read_success || bytes_read != 512) {
            goto read_retry;
        }

        if (msc_receive_csw() != RES_OK) {
            goto read_retry;
        }

        if (buff) {
            if (offset >= 512 || (offset + count) > 512) {
                return RES_PARERR;
            }
            memcpy(buff, &sector_buffer[offset], count);
        }

        // Tiny pacing delay between sector reads to avoid overwhelming some devices
        for (volatile int i = 0; i < 800; i++);
        return RES_OK;
        
read_retry:
        // Try graduated recovery: sense on early attempts, then full reset
        msc_request_sense(sense_data);
        // If sense indicates Not Ready or Unit Attention, pause a bit longer
        if (((sense_data[2] & 0x0F) == 0x02) /* Not Ready */ || ((sense_data[2] & 0x0F) == 0x06) /* Unit Attention */) {
            for (volatile int i = 0; i < 20000; i++);
        }
        msc_mass_storage_reset();
        // Extra settle time before retrying the entire READ(10)
        for (volatile int i = 0; i < 10000; i++);
    }
    
    return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/
DRESULT disk_writep(BYTE* buff, DWORD sc)
{
    DRESULT res = RES_OK;

    if (!buff) {
        if (sc) {
            current_write_sector = sc;
            write_buffer_offset = 0;
            write_operation_active = 1;
            memset(write_sector_buffer, 0, sizeof(write_sector_buffer));
        } else {
            if (write_operation_active && write_buffer_offset > 0) {
                res = msc_write_sector(current_write_sector, write_sector_buffer);
                if (res != RES_OK) {
                    write_operation_active = 0;
                    return res;
                }
            }
            
            write_operation_active = 0;
            write_buffer_offset = 0;
            current_write_sector = 0;
        }
    } else {
        if (!write_operation_active) {
            return RES_ERROR;
        }
        
        UINT bytes_to_write = (UINT)sc;
        
        if ((write_buffer_offset + bytes_to_write) > 512) {
            res = msc_write_sector(current_write_sector, write_sector_buffer);
            if (res != RES_OK) {
                write_operation_active = 0;
                return res;
            }
            
            current_write_sector++;
            write_buffer_offset = 0;
            memset(write_sector_buffer, 0, sizeof(write_sector_buffer));
            
            if (bytes_to_write > 512) {
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
        
        if (bytes_to_write > 0) {
            memcpy(&write_sector_buffer[write_buffer_offset], buff, bytes_to_write);
            write_buffer_offset += bytes_to_write;
        }
    }

    return res;
}