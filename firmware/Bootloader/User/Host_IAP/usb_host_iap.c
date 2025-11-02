/********************************** (C) COPYRIGHT  *******************************
 * File Name          : iap.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/20
 * Description        : IAP
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "usb_host_iap.h"
#include "utils.h"
#include "pff.h"
#include "diskio.h"

/* Variable */
__attribute__ ((aligned (4))) uint8_t IAPLoadBuffer[DEF_MAX_IAP_BUFFER_LEN];
__attribute__ ((aligned (4))) uint8_t Com_Buffer[DEF_COM_BUF_LEN];  // even address , used for host enumcation and udisk operation
__attribute__ ((aligned (4))) uint8_t DevDesc_Buf[18];              // Device Descriptor Buffer
volatile uint32_t IAP_Load_Addr_Offset;
volatile uint32_t IAP_WriteIn_Length;
volatile uint32_t IAP_WriteIn_Count;
struct _ROOT_HUB_DEVICE RootHubDev[DEF_TOTAL_ROOT_HUB];
struct __HOST_CTL HostCtl[DEF_TOTAL_ROOT_HUB * DEF_ONE_USB_SUP_DEV_TOTAL];

/* PFF File System Variables */
FATFS pff_fs;                      // PFF file system object
volatile uint32_t pff_file_size;   // Current file size
volatile uint8_t pff_disk_status;  // Disk status: 0=not ready, 1=ready, 2=mounted

/* Flash Operation Key */
volatile uint32_t Flash_Operation_Key0;
volatile uint32_t Flash_Operation_Key1;

/*********************************************************************
 * @fn      FLASH_ReadByte
 *
 * @brief   Read Flash In byte(8 bits)
 *
 * @return  8bits data readout
 */
uint8_t FLASH_ReadByte (uint32_t address) {
    return *(__IO uint8_t *)address;
}

/*********************************************************************
 * @fn      FLASH_ReadHalfWord
 *
 * @brief   Read Flash In HalfWord(16 bits)
 *
 * @return  16bits data readout
 */
uint16_t FLASH_ReadHalfWord (uint32_t address) {
    return *(__IO uint16_t *)address;
}

/*********************************************************************
 * @fn      FLASH_ReadWord
 *
 * @brief   Read Flash In Word(32 bits).
 *
 * @return  32bits data readout
 */
uint32_t FLASH_ReadWord (uint32_t address) {
    return *(__IO uint32_t *)address;
}

/*********************************************************************
 * @fn      FLASH_ReadWordAdd
 *
 * @brief   Read Flash In Word(32 bits),Specify length & address
 *
 * @return  none
 */
void FLASH_ReadWordAdd (uint32_t address, u32 *buff, uint16_t length) {
    uint16_t i;

    for (i = 0; i < length; i++) {
        buff[i] = *(__IO uint32_t *)address;
        address += 4;
    }
}

/*********************************************************************
 * @fn      IAP_Flash_Read
 *
 * @brief   Read Flash In bytes(8 bits),Specify length & address,
 *          With address protection and program runaway protection.
 *
 * @return  0: Operation Success
 *          See notes for other errors
 */
uint8_t IAP_Flash_Read (uint32_t address, uint8_t *buff, uint32_t length) {
    uint32_t i;
    uint32_t read_addr;
    uint32_t read_len;
    read_addr = address;
    read_len = length;

    /* Verify Keys, No flash operation if keys are not correct */
    if ((Flash_Operation_Key0 != DEF_FLASH_OPERATION_KEY_CODE_0) || (Flash_Operation_Key1 != DEF_FLASH_OPERATION_KEY_CODE_1)) {
        /* ERR: Risk of code running away */
        return 0xFF;
    }

    /* Verify Address, No flash operation if the address is out of range */
    if (((read_addr >= DEF_APP_CODE_START_ADDR) && (read_addr <= DEF_APP_CODE_END_ADDR))) {
        /* Available Data */
        for (i = 0; i < read_len; i++) {
            buff[i] = *(__IO uint8_t *)read_addr;  // Read one word of data at the specified address
            read_addr++;
        }
        if (i != read_len) {
            /* Incorrect read length */
            return 0xFD;
        }
    } else {
        /* address Out Of Range */
        return 0xFE;
    }

    return 0;
}

/*********************************************************************
 * @fn      mFLASH_ProgramPage_Fast
 *
 * @brief   Fast programming, input variable addresses and data buffers,
 *          no longer than one page at a time.
 *          The content of the functions can be written according to the
 *          chip flash fast programming process by yourself
 *
 * @return  0: Operation Success
 *          See notes for other errors
 */
uint8_t mFLASH_ProgramPage_Fast (uint32_t addr, uint32_t *buffer) {
    FLASH_ProgramPage_Fast (addr, buffer);
    return 0;
}

/*********************************************************************
 * @fn      IAP_Flash_Write
 *
 * @brief   Write Flash In bytes(8 bits),Specify length & address,
 *          Based On Fast Flash Operation,
 *          With address protection and program runaway protection.
 *
 * @return  0: Operation Success
 *          See notes for other errors
 */
uint8_t IAP_Flash_Write (uint32_t address, uint8_t *buff, uint32_t length) {
    uint32_t i, j;
    uint32_t write_addr;
    uint32_t write_len;
    uint16_t write_len_once;
    uint32_t write_cnts;
    volatile uint32_t page_cnts;
    volatile uint32_t page_addr;
    uint8_t temp_buf[DEF_FLASH_PAGE_SIZE];

    /* Set initial value */
    write_addr = address;
    page_addr = address;
    write_len = length;
    if ((write_len % DEF_FLASH_PAGE_SIZE) == 0) {
        page_cnts = write_len / DEF_FLASH_PAGE_SIZE;
    } else {
        page_cnts = (write_len / DEF_FLASH_PAGE_SIZE) + 1;
    }
    write_cnts = 0;

    /* Verify Keys, No flash operation if keys are not correct */
    if ((Flash_Operation_Key0 != DEF_FLASH_OPERATION_KEY_CODE_0) || (Flash_Operation_Key1 != DEF_FLASH_OPERATION_KEY_CODE_1)) {
        /* ERR: Risk of code running away */
        return 0xFF;
    }

    /* Verify Address, No flash operation if the address is out of range */
    if (((write_addr >= DEF_APP_CODE_START_ADDR) && (write_addr <= DEF_APP_CODE_END_ADDR))) {
        for (i = 0; i < page_cnts; i++) {
            /* Verify Keys, No flash operation if keys are not correct */
            if (Flash_Operation_Key0 != DEF_FLASH_OPERATION_KEY_CODE_0) {
                /* ERR: Risk of code running away */
                return 0xFF;
            }
            /* Determine if the length of the written packet is less than the page size */
            /* If it is less than, the original content of the memory needs to be read out first and modified before the operation can be performed */
            write_len_once = DEF_FLASH_PAGE_SIZE;
            FLASH_Unlock_Fast();
            FLASH_ErasePage_Fast (page_addr);
            if (write_len < DEF_FLASH_PAGE_SIZE) {
                FLASH_Unlock_Fast();
                IAP_Flash_Read (page_addr, temp_buf, DEF_FLASH_PAGE_SIZE);

                for (j = 0; j < write_len; j++) {
                    temp_buf[j] = buff[DEF_FLASH_PAGE_SIZE * i + j];
                }
                write_len_once = write_len;
                mFLASH_ProgramPage_Fast (page_addr, (u32 *)&temp_buf[0]);
            } else {
                mFLASH_ProgramPage_Fast (page_addr, (u32 *)&buff[DEF_FLASH_PAGE_SIZE * i]);
            }
            page_addr += DEF_FLASH_PAGE_SIZE;
            write_len -= write_len_once;
            write_cnts++;
        }
        if (i != write_cnts) {
            /* Incorrect read length */
            return 0xFD;
        }
        FLASH_Lock_Fast();
    } else {
        /* address Out Of Range */
        return 0xFE;
    }

    return 0;
}

/*********************************************************************
 * @fn      IAP_Flash_Erase
 *
 * @brief   Erase Flash In page(256 bytes),Specify length & address,
 *          Based On Fast Flash Operation,
 *          With address protection and program runaway protection.
 *
 * @return  0: Operation Success
 *          See notes for other errors
 */
uint8_t IAP_Flash_Erase (uint32_t address, uint32_t length) {
    uint32_t i;
    uint32_t erase_addr;
    uint32_t erase_len;
    volatile uint32_t page_cnts;
    volatile uint32_t page_addr;

    /* Set initial value */
    erase_addr = address;
    page_addr = address;
    erase_len = length;
    if ((erase_len % DEF_FLASH_PAGE_SIZE) == 0) {
        page_cnts = erase_len / DEF_FLASH_PAGE_SIZE;
    } else {
        page_cnts = (erase_len / DEF_FLASH_PAGE_SIZE) + 1;
    }

    /* Verify Keys, No flash operation if keys are not correct */
    if ((Flash_Operation_Key0 != DEF_FLASH_OPERATION_KEY_CODE_0) || (Flash_Operation_Key1 != DEF_FLASH_OPERATION_KEY_CODE_1)) {
        /* ERR: Risk of code running away */
        return 0xFF;
    }
    /* Verify Address, No flash operation if the address is out of range */
    if (((erase_addr >= DEF_APP_CODE_START_ADDR) && (erase_addr <= DEF_APP_CODE_END_ADDR))) {
        for (i = 0; i < page_cnts; i++) {
            /* Verify Keys, No flash operation if keys are not correct */
            if (Flash_Operation_Key0 != DEF_FLASH_OPERATION_KEY_CODE_0) {
                /* ERR: Risk of code running away */
                return 0xFF;
            }
            FLASH_Unlock_Fast();
            FLASH_ErasePage_Fast (page_addr);
            page_addr += DEF_FLASH_PAGE_SIZE;
        }
    }

    return 0;
}

/*********************************************************************
 * @fn      IAP_Flash_Verify
 *
 * @brief   verify Flash In bytes(8 bits),Specify length & address,
 *          With address protection and program runaway protection.
 *
 * @return  0: Operation Success
 *          See notes for other errors
 */
uint32_t IAP_Flash_Verify (uint32_t address, uint8_t *buff, uint32_t length) {
    uint32_t i;
    uint32_t read_addr;
    uint32_t read_len;

    /* Set initial value */
    read_addr = address;
    read_len = length;

    /* Verify Keys, No flash operation if keys are not correct */
    if ((Flash_Operation_Key0 != DEF_FLASH_OPERATION_KEY_CODE_0) || (Flash_Operation_Key1 != DEF_FLASH_OPERATION_KEY_CODE_1)) {
        /* ERR: Risk of code running away */
        return 0xFFFFFFFF;
    }

    /* Verify Address, No flash operation if the address is out of range */
    if (((read_addr >= DEF_APP_CODE_START_ADDR) && (read_addr <= DEF_APP_CODE_END_ADDR))) {
        for (i = 0; i < read_len; i++) {
            if (FLASH_ReadByte (read_addr) != buff[i]) {
                /* To prevent 0-length errors, the returned position +1 */
                return i + 1;
            }
            read_addr++;
        }
    }

    return 0;
}

/*********************************************************************
 * @fn      IAP_Flash_Program
 *
 * @brief   IAP programming code, including write and verify,
 *          Specify length & address, Based On Fast Flash Operation,
 *          With address protection and program runaway protection.
 *
 * @return  ret : The meaning of 'ret' can be found in the notes of the
 *          corresponding function.
 */
uint32_t IAP_Flash_Program (uint32_t address, uint8_t *buff, uint32_t length) {
    uint32_t ret;
    /* IAP Write */
    Flash_Operation_Key1 = DEF_FLASH_OPERATION_KEY_CODE_1;
    ret = IAP_Flash_Write (address, buff, length);
    Flash_Operation_Key1 = 0;
    if (ret != 0) {
        return ret;
    }
    /* IAP Verify */
    Flash_Operation_Key1 = DEF_FLASH_OPERATION_KEY_CODE_1;
    ret = IAP_Flash_Verify (address, buff, length);
    Flash_Operation_Key1 = 0;
    if (ret != 0) {
        return ret;
    }

    return ret;
}

/*********************************************************************
 * @fn      IAP_Jump_APP
 *
 * @brief   Start the Operation of jumping to user application
 *
 * @return  none
 */
void IAP_Jump_APP (void) {
    /* Best-effort deinit of peripherals we touched */
    DeInitStuff();

    /* Enable the software interrupt and trigger it */
    NVIC_EnableIRQ(Software_IRQn);
    __enable_irq();  // Ensure global interrupts are enabled
    NVIC_SetPendingIRQ(Software_IRQn);
    
    /* Wait for interrupt to fire - it will jump to app */
    while(1) {
        __NOP();
    }
}

/*********************************************************************
 * @fn      PFF_Check_And_Mount
 *
 * @brief   Check if disk is ready and mount PFF filesystem
 *
 * @return  0: Success, Others: Error
 */
uint8_t PFF_Check_And_Mount (void) {
    FRESULT res;
    DSTATUS disk_stat;

    // Check disk status
    disk_stat = disk_initialize();
    if (disk_stat & STA_NOINIT) {
        return 0x01;  // Disk not initialized
    }
    if (disk_stat & STA_NODISK) {
        return 0x02;  // No disk
    }

    // Try to mount filesystem
    res = pf_mount (&pff_fs);
    if (res == FR_OK) {
        pff_disk_status = 2;  // Mounted
        return 0;
    } else if (res == FR_NOT_READY) {
        pff_disk_status = 0;  // Not ready
        return 0x02;
    } else {
        pff_disk_status = 0;  // Error
        return 0x03;
    }
}

/*********************************************************************
 * @fn      mStopIfError
 *
 * @brief   Checking the operation status, displaying the error code and stopping if there is an error
 *          input : iError - Error code input
 *
 * @return  none
 */
void mStopIfError (uint8_t iError) {
    if (iError == ERR_SUCCESS) {
        /* operation success, return */
        return;
    }
    /* Display the errors */
    DUG_PRINTF ("Error:%02x\r\n", iError);
    /* After encountering an error, you should analyze the error code and disk status, for example,
     * check whether the current USB disk is connected or not,
     * if the disk is disconnected then wait for the disk to be plugged in again and operate again,
     * Suggested steps to follow after an error:
     *     1, check disk status once, if successful, then continue the operation, such as Open, Read/Write, etc.
     *     2. If disk is not ready, then the operation will be forced to start from the beginning.
     */
    while (1) { }
}

/*********************************************************************
 * @fn      IAP_Initialization
 *
 * @brief   IAP process Initialization, include usb-host initialization
 *          usb libs initialization, iap-related values Initialization
 *          IAP verify-code inspection
 *
 * @return  none
 */
void IAP_Initialization (void) {


    /* USB Host Initialization */
    DUG_PRINTF ("USB Host & UDisk Lib Initialization. \r\n");
    /* Initialize USBFS host */
    DUG_PRINTF ("USBFS Host Init\r\n");
    USBFS_RCC_Init();
    USBFS_Host_Init (ENABLE);
    memset (&RootHubDev[DEF_USB_PORT_FS].bStatus, 0, sizeof (struct _ROOT_HUB_DEVICE));
    memset (&HostCtl[DEF_USB_PORT_FS].InterfaceNum, 0, sizeof (struct __HOST_CTL));

    /* USB Libs Initialization */
    DUG_PRINTF ("PFF library Initialization. \r\n");
    // PFF will be initialized when disk is ready

    /* IAP-related variable initialization  */
    IAP_Load_Addr_Offset = 0;
    IAP_WriteIn_Length = 0;
    IAP_WriteIn_Count = 0;
    pff_disk_status = 0;
    pff_file_size = 0;
}

/*********************************************************************
 * @fn      USBH_EnumRootDevice
 *
 * @brief   Generally enumerate a device connected to host port.
 *
 * @para    index: USB host port
 *
 * @return  Enumeration result
 */
uint8_t USBH_EnumRootDevice (uint8_t usb_port) {
    uint8_t s;
    uint8_t enum_cnt;
    uint8_t cfg_val;
    uint16_t i;
    uint16_t len;

    DUG_PRINTF ("Enum:\r\n");

    enum_cnt = 0;
ENUM_START:
    /* Delay and wait for the device to stabilize */
    Delay_Ms (100);
    enum_cnt++;
    Delay_Ms (8 << enum_cnt);

    /* Reset the USB device and wait for the USB device to reconnect */
    USBFSH_ResetRootHubPort (0);
    for (i = 0, s = 0; i < DEF_RE_ATTACH_TIMEOUT; i++) {
        if (USBFSH_EnableRootHubPort (&RootHubDev[usb_port].bSpeed) == ERR_SUCCESS) {
            i = 0;
            s++;
            if (s > 6) {
                break;
            }
        }
        Delay_Ms (1);
    }
    if (i) {
        /* Determine whether the maximum number of retries has been reached, and retry if not reached */
        if (enum_cnt <= 5) {
            goto ENUM_START;
        }
        return ERR_USB_DISCON;
    }

    /* Select USB speed */
    USBFSH_SetSelfSpeed (RootHubDev[usb_port].bSpeed);

    /* Get USB device device descriptor */
    DUG_PRINTF ("Get DevDesc: ");
    s = USBFSH_GetDeviceDescr (&RootHubDev[usb_port].bEp0MaxPks, DevDesc_Buf);
    if (s == ERR_SUCCESS) {
        /* Print USB device device descriptor */
#if DEF_DEBUG_PRINTF
        for (i = 0; i < 18; i++) {
            DUG_PRINTF ("%02x ", DevDesc_Buf[i]);
        }
        DUG_PRINTF ("\n");
#endif
    } else {
        /* Determine whether the maximum number of retries has been reached, and retry if not reached */
        DUG_PRINTF ("Err(%02x)\n", s);
        if (enum_cnt <= 5) {
            goto ENUM_START;
        }
        return DEF_DEV_DESCR_GETFAIL;
    }

    /* Set the USB device address */
    DUG_PRINTF ("Set DevAddr: ");
    RootHubDev[usb_port].bAddress = (uint8_t)(DEF_USB_PORT_FS + USB_DEVICE_ADDR);
    s = USBFSH_SetUsbAddress (RootHubDev[usb_port].bEp0MaxPks, RootHubDev[usb_port].bAddress);
    if (s == ERR_SUCCESS) {
        DUG_PRINTF ("OK\n");
    } else {
        /* Determine whether the maximum number of retries has been reached, and retry if not reached */
        DUG_PRINTF ("Err(%02x)\n", s);
        if (enum_cnt <= 5) {
            goto ENUM_START;
        }
        return DEF_DEV_ADDR_SETFAIL;
    }
    Delay_Ms (5);

    /* Get the USB device configuration descriptor */
    DUG_PRINTF ("Get CfgDesc: ");
    s = USBFSH_GetConfigDescr (RootHubDev[usb_port].bEp0MaxPks, Com_Buffer, DEF_COM_BUF_LEN, &len);
    if (s == ERR_SUCCESS) {
        cfg_val = ((PUSB_CFG_DESCR)Com_Buffer)->bConfigurationValue;

        /* Print USB device configuration descriptor  */
#if DEF_DEBUG_PRINTF
        for (i = 0; i < len; i++) {
            DUG_PRINTF ("%02x ", Com_Buffer[i]);
        }
        DUG_PRINTF ("\n");
#endif
    } else {
        /* Determine whether the maximum number of retries has been reached, and retry if not reached */
        DUG_PRINTF ("Err(%02x)\n", s);
        if (enum_cnt <= 5) {
            goto ENUM_START;
        }
        return DEF_CFG_DESCR_GETFAIL;
    }

    /* Set USB device configuration value */
    DUG_PRINTF ("Set Cfg: ");
    s = USBFSH_SetUsbConfig (RootHubDev[usb_port].bEp0MaxPks, cfg_val);
    if (s == ERR_SUCCESS) {
        DUG_PRINTF ("OK\n");

        /* Parse configuration descriptor to populate HostCtl structure */
        IAP_Parse_Config_Descriptor (usb_port, Com_Buffer, len);

    } else {
        /* Determine whether the maximum number of retries has been reached, and retry if not reached */
        DUG_PRINTF ("Err(%02x)\n", s);
        if (enum_cnt <= 5) {
            goto ENUM_START;
        }
        return ERR_USB_UNSUPPORT;
    }

    return ERR_SUCCESS;
}

/*********************************************************************
 * @fn      IAP_USBH_PreDeal
 *
 * @brief   usb host preemption operations,
 *         including detecting device insertion and enumerating device information
 *
 * @return  none
 */
uint8_t IAP_USBH_PreDeal (void) {
    uint8_t usb_port;
    uint8_t index;
    uint8_t ret;
    usb_port = DEF_USB_PORT_FS;
    ret = USBFSH_CheckRootHubPortStatus (RootHubDev[usb_port].bStatus);
    if (ret == ROOT_DEV_CONNECTED) {
        DUG_PRINTF ("USB Dev In.\n");
        USBFSH_CheckRootHubPortStatus (RootHubDev[usb_port].bStatus);
        RootHubDev[usb_port].bStatus = ROOT_DEV_CONNECTED;  // Set connection status_
        RootHubDev[usb_port].DeviceIndex = usb_port * DEF_ONE_USB_SUP_DEV_TOTAL;

        /* Enumerate root device */
        ret = USBH_EnumRootDevice (usb_port);
        if (ret == ERR_SUCCESS) {
            DUG_PRINTF ("USB Port %02x Device Enumeration Succeed\r\n", usb_port);
            RootHubDev[usb_port].bStatus = ROOT_DEV_SUCCESS;
            return DEF_IAP_SUCCESS;
        } else {
            DUG_PRINTF ("USB Port %02x Device Enumeration ERR %02x.\r\n", usb_port, ret);
            RootHubDev[usb_port].bStatus = ROOT_DEV_FAILED;
            return DEF_IAP_ERR_ENUM;
        }
    } else if (ret == ROOT_DEV_DISCONNECT) {
        DUG_PRINTF ("USB Port %02x Device Out.\r\n", usb_port);
        /* Clear parameters */
        index = RootHubDev[usb_port].DeviceIndex;
        memset (&RootHubDev[usb_port].bStatus, 0, sizeof (struct _ROOT_HUB_DEVICE));
        memset (&HostCtl[index].InterfaceNum, 0, sizeof (struct __HOST_CTL));
        return DEF_IAP_ERR_DETECT;
    }
    return DEF_IAP_DEFAULT;
}

/*********************************************************************
 * @fn      IAP_Main_Deal
 *
 * @brief   IAP Transaction Processing
 *
 * @return  none
 */
void IAP_Main_Deal (void) {
    uint16_t i, ret;
    static uint8_t op_flag = 0;

    /* Detect USB Device & Enumeration processing */
    ret = IAP_USBH_PreDeal();
    if (ret == DEF_IAP_SUCCESS) {
        /* Wait for uDisk Ready and Mount PFF */
        pff_disk_status = 0;
        for (i = 0; i != 10; i++) {
            ret = PFF_Check_And_Mount();
            if (ret == 0) {
                op_flag = 1;
                break;
            }
            Delay_Ms (50);
        }
    }

    if ((pff_disk_status >= 2) && op_flag)  // 2 = mounted
    {
        op_flag = 0;
        /* Make sure the flash operation is correct */
        Flash_Operation_Key0 = DEF_FLASH_OPERATION_KEY_CODE_0;

        /* Try to open the IAP file */
        FRESULT pf_res = pf_open (DEF_IAP_FILE_NAME);

        /* file not found */
        if (pf_res == FR_NO_FILE) {
            DUG_PRINTF ("File not found\r\n");
        }
        /* Found file, start IAP processing */
        else if (pf_res == FR_OK) {
            pff_file_size = pff_fs.fsize;
            DUG_PRINTF ("IAP: %d bytes\r\n", (int)pff_file_size);

            /* Make sure the flash operation is correct */
            Flash_Operation_Key0 = DEF_FLASH_OPERATION_KEY_CODE_0;
            IAP_Load_Addr_Offset = 0;
            IAP_WriteIn_Length = 0;
            IAP_WriteIn_Count = 0;

            /* Binary file read & iap write in */
            uint32_t totalcount = pff_file_size;
            uint32_t total_read = 0;

            while (totalcount > 0) {
                /* Determine read size - limit to available buffer space and use smaller chunks for USB stability */
                uint16_t read_size;
                uint16_t buffer_space = DEF_MAX_IAP_BUFFER_LEN - IAP_WriteIn_Length;
                uint16_t max_chunk_size = 512;  // Limit to 512 bytes per read for USB stability

                if (totalcount > buffer_space) {
                    read_size = buffer_space;
                } else {
                    read_size = (uint16_t)totalcount;
                }

                /* Further limit chunk size for USB stability */
                if (read_size > max_chunk_size) {
                    read_size = max_chunk_size;
                }

                /* Read data directly into IAP buffer with retry */
                UINT bytes_read = 0;
                uint8_t retry_count = 0;
                const uint8_t max_retries = 3;

                do {
                    pf_res = pf_read (&IAPLoadBuffer[IAP_WriteIn_Length], read_size, &bytes_read);

                    if (pf_res == FR_OK) {
                        break;
                    }

                    if (pf_res == FR_DISK_ERR) {
                        Delay_Ms (200);
                        disk_reset_state();
                        pff_disk_status = 0;

                        uint8_t mount_attempts = 0;
                        while (mount_attempts < 3) {
                            ret = PFF_Check_And_Mount();
                            if (ret == 0) {
                                pf_res = pf_open (DEF_IAP_FILE_NAME);
                                if (pf_res == FR_OK) {
                                    pf_res = pf_lseek (total_read);
                                    if (pf_res == FR_OK) {
                                        break;
                                    }
                                }
                            }
                            mount_attempts++;
                            Delay_Ms (300);
                        }

                        if (mount_attempts >= 3) {
                            break;
                        }
                    }

                    retry_count++;
                    if (retry_count < max_retries) {
                        Delay_Ms (100);
                    }

                } while (retry_count < max_retries);

                if (pf_res != FR_OK) {
                    DUG_PRINTF ("Read err: %d\r\n", pf_res);
                    break;
                }

                if (bytes_read == 0) {
                    break;
                }

                /* Add small delay between successful reads to prevent USB overload */
                if (total_read > 0 && (total_read % (4 * DEF_MAX_IAP_BUFFER_LEN)) == 0) {
                    Delay_Ms (10);
                }

                IAP_WriteIn_Length += bytes_read;
                totalcount -= bytes_read;
                total_read += bytes_read;

                /* Write buffer when full or at end of file */
                if (IAP_WriteIn_Length == DEF_MAX_IAP_BUFFER_LEN || totalcount == 0) {
                    uint32_t write_addr = DEF_APP_CODE_START_ADDR + IAP_Load_Addr_Offset;
                    
                    ret = IAP_Flash_Program (write_addr, IAPLoadBuffer, IAP_WriteIn_Length);
                    if (ret != 0) {
                        DUG_PRINTF ("Flash err @0x%08X\r\n", (unsigned int)write_addr);
                        break;
                    }
                    IAP_Load_Addr_Offset += IAP_WriteIn_Length;
                    IAP_WriteIn_Count += IAP_WriteIn_Length;

                    IAP_WriteIn_Length = 0;
                }
            }

            if (pff_file_size == IAP_WriteIn_Count) {
                DUG_PRINTF ("Verifying...\r\n");
                
                /* Re-open file for verification */
                pf_res = pf_open (DEF_IAP_FILE_NAME);
                if (pf_res != FR_OK) {
                    DUG_PRINTF ("Vfy open: %d\r\n", pf_res);
                    blinkLed (100, 100);
                } else {
                    uint32_t vfy_off = 0;
                    uint32_t vfy_rem = pff_file_size;
                    uint32_t vfy_err = 0;
                    
                    while (vfy_rem > 0 && vfy_err == 0) {
                        uint16_t chunk = (vfy_rem > DEF_MAX_IAP_BUFFER_LEN) ? 
                                         DEF_MAX_IAP_BUFFER_LEN : (uint16_t)vfy_rem;
                        
                        UINT rd = 0;
                        pf_res = pf_read (IAPLoadBuffer, chunk, &rd);
                        if (pf_res != FR_OK || rd != chunk) {
                            vfy_err = 1;
                            break;
                        }
                        
                        Flash_Operation_Key1 = DEF_FLASH_OPERATION_KEY_CODE_1;
                        uint32_t faddr = DEF_APP_CODE_START_ADDR + vfy_off;
                        for (uint32_t i = 0; i < rd; i++) {
                            if (FLASH_ReadByte(faddr + i) != IAPLoadBuffer[i]) {
                                DUG_PRINTF ("Vfy fail @0x%08X\r\n", (unsigned int)(faddr + i));
                                vfy_err = 1;
                                break;
                            }
                        }
                        Flash_Operation_Key1 = 0;
                        
                        vfy_off += rd;
                        vfy_rem -= rd;
                    }
                    
                    if (vfy_err) {
                        DUG_PRINTF ("Vfy FAIL\r\n");
                        blinkLed (100, 100);
                    } else {
                        DUG_PRINTF ("Vfy OK\r\n");
                        blinkLed (10, 500);
                        IAP_Jump_APP();
                    }
                }
            } else {
                DUG_PRINTF ("Len err\r\n");
                blinkLed (100, 100);
            }
        } else {
            DUG_PRINTF ("Open err: %d\r\n", pf_res);
        }
    }
}

/*********************************************************************
 * @fn      IAP_Parse_Config_Descriptor
 *
 * @brief   Parse USB configuration descriptor and populate HostCtl structure
 *
 * @param   usb_port - USB port number
 * @param   desc_buf - Configuration descriptor buffer
 * @param   desc_len - Descriptor length
 *
 * @return  none
 */
void IAP_Parse_Config_Descriptor (uint8_t usb_port, uint8_t *desc_buf, uint16_t desc_len) {
    uint8_t *p = desc_buf;
    uint8_t *end = desc_buf + desc_len;
    uint8_t desc_type, desc_length;
    uint8_t interface_num = 0;
    uint8_t current_interface = 0xFF;

    DUG_PRINTF ("Parsing config descriptor (%d bytes)...\r\n", desc_len);

    // Clear HostCtl structure
    memset (&HostCtl[usb_port], 0, sizeof (struct __HOST_CTL));

    while (p < end && interface_num < DEF_INTERFACE_NUM_MAX) {
        desc_length = p[0];
        desc_type = p[1];

        if (desc_length < 2 || p + desc_length > end) {
            break;  // Invalid descriptor
        }

        switch (desc_type) {
        case USB_DESCR_TYP_INTERF:  // Interface descriptor
            if (desc_length >= 9) {
                current_interface = interface_num;
                HostCtl[usb_port].Interface[interface_num].Type = p[5];  // bInterfaceClass
                DUG_PRINTF ("Interface %d: Class=0x%02x, SubClass=0x%02x, Protocol=0x%02x\r\n",
                            interface_num, p[5], p[6], p[7]);
                interface_num++;
            }
            break;

        case USB_DESCR_TYP_ENDP:  // Endpoint descriptor
            if (desc_length >= 7 && current_interface < DEF_INTERFACE_NUM_MAX) {
                uint8_t ep_addr = p[2];
                uint8_t ep_attr = p[3];
                uint16_t ep_size = p[4] | (p[5] << 8);

                if ((ep_addr & 0x80) == 0) {  // OUT endpoint
                    uint8_t out_idx = HostCtl[usb_port].Interface[current_interface].OutEndpNum;
                    if (out_idx < 4) {
                        HostCtl[usb_port].Interface[current_interface].OutEndpAddr[out_idx] = ep_addr;
                        HostCtl[usb_port].Interface[current_interface].OutEndpType[out_idx] = ep_attr & 0x03;
                        HostCtl[usb_port].Interface[current_interface].OutEndpSize[out_idx] = ep_size;
                        HostCtl[usb_port].Interface[current_interface].OutEndpNum++;
                        DUG_PRINTF ("OUT EP: Addr=0x%02x, Type=%d, Size=%d\r\n", ep_addr, ep_attr & 0x03, ep_size);
                    }
                } else {  // IN endpoint
                    uint8_t in_idx = HostCtl[usb_port].Interface[current_interface].InEndpNum;
                    if (in_idx < 4) {
                        HostCtl[usb_port].Interface[current_interface].InEndpAddr[in_idx] = ep_addr;
                        HostCtl[usb_port].Interface[current_interface].InEndpType[in_idx] = ep_attr & 0x03;
                        HostCtl[usb_port].Interface[current_interface].InEndpSize[in_idx] = ep_size;
                        HostCtl[usb_port].Interface[current_interface].InEndpNum++;
                        DUG_PRINTF ("IN EP: Addr=0x%02x, Type=%d, Size=%d\r\n", ep_addr, ep_attr & 0x03, ep_size);
                    }
                }
            }
            break;

        default:
            // Skip other descriptor types
            break;
        }

        p += desc_length;
    }

    HostCtl[usb_port].InterfaceNum = interface_num;
    DUG_PRINTF ("Found %d interfaces\r\n", interface_num);
}

/*********************************************************************
 * @fn      IAP_Get_USB_Status
 *
 * @brief   Get USB device status for a specific port
 *
 * @param   port - USB port number
 *
 * @return  USB device status
 */
uint8_t IAP_Get_USB_Status (uint8_t port) {
    if (port >= DEF_TOTAL_ROOT_HUB) {
        return 0;  // Invalid port
    }
    return RootHubDev[port].bStatus;
}
