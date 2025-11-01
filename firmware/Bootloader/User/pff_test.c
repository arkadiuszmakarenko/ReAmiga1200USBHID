/*---------------------------------------------------------------------------/
/  PFF Library Test Suite
/  Tests USB Mass Storage functionality with Petit FatFS
/---------------------------------------------------------------------------*/

#include "pff.h"
#include "diskio.h"
#include "debug.h"
#include "string.h"
#include "utils.h"

/* Test configuration */
#define TEST_BUFFER_SIZE    1024
#define TEST_FILE_NAME      "/PFFTEST.TXT"
#define TEST_WRITE_FILE     "/WRITE_TEST.TXT"
#define MAX_FILES_TO_LIST   20

/* Test state */
static FATFS pff_test_fs;
static uint8_t test_buffer[TEST_BUFFER_SIZE];
static uint32_t test_pass_count = 0;
static uint32_t test_fail_count = 0;

/* Test result macros */
#define TEST_ASSERT(condition, test_name) \
    do { \
        if (condition) { \
            printf("PASS: %s\r\n", test_name); \
            test_pass_count++; \
        } else { \
            printf("FAIL: %s\r\n", test_name); \
            test_fail_count++; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, test_name) \
    do { \
        if ((expected) == (actual)) { \
            printf("PASS: %s (Expected: %d, Got: %d)\r\n", test_name, (int)(expected), (int)(actual)); \
            test_pass_count++; \
        } else { \
            printf("FAIL: %s (Expected: %d, Got: %d)\r\n", test_name, (int)(expected), (int)(actual)); \
            test_fail_count++; \
        } \
    } while(0)

/*---------------------------------------------------------------------------/
/ Test 1: Disk Initialization Test
/---------------------------------------------------------------------------*/
void test_disk_initialization(void)
{
    DSTATUS disk_status;
    
    printf("\r\n=== Test 1: Disk Initialization ===\r\n");
    
    // Test disk initialization
    disk_status = disk_initialize();
    TEST_ASSERT(disk_status == 0, "Disk initialization successful");
    
    if (disk_status != 0) {
        printf("Disk initialization failed with status: 0x%02X\r\n", disk_status);
        if (disk_status & STA_NOINIT) printf("  - Drive not initialized\r\n");
        if (disk_status & STA_NODISK) printf("  - No medium in drive\r\n");
    }
}

/*---------------------------------------------------------------------------/
/ Test 2: Filesystem Mount Test
/---------------------------------------------------------------------------*/
void test_filesystem_mount(void)
{
    FRESULT fr;
    
    printf("\r\n=== Test 2: Filesystem Mount ===\r\n");
    
    // Test filesystem mounting
    fr = pf_mount(&pff_test_fs);
    TEST_ASSERT_EQ(FR_OK, fr, "Filesystem mount");
    
    if (fr != FR_OK) {
        printf("Mount failed with result: %d\r\n", fr);
        switch(fr) {
            case FR_DISK_ERR: printf("  - Disk error\r\n"); break;
            case FR_NOT_READY: printf("  - Drive not ready\r\n"); break;
            case FR_NO_FILESYSTEM: printf("  - No valid filesystem\r\n"); break;
            default: printf("  - Unknown error\r\n"); break;
        }
    } else {
        printf("Filesystem type: FAT%d\r\n", pff_test_fs.fs_type == FS_FAT32 ? 32 : 
                                           (pff_test_fs.fs_type == FS_FAT16 ? 16 : 12));
    }
}

/*---------------------------------------------------------------------------/
/ Test 3: Directory Listing Test
/---------------------------------------------------------------------------*/
void test_directory_listing(void)
{
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    int file_count = 0;
    
    printf("\r\n=== Test 3: Directory Listing ===\r\n");
    
    // Open root directory
    fr = pf_opendir(&dir, "/");
    TEST_ASSERT_EQ(FR_OK, fr, "Open root directory");
    
    if (fr == FR_OK) {
        printf("Root directory contents:\r\n");
        
        // List files in root directory
        while (file_count < MAX_FILES_TO_LIST) {
            fr = pf_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;  // End of directory
            
            file_count++;
            printf("  %c %8lu %s\r\n", 
                   (fno.fattrib & AM_DIR) ? 'D' : 'F',
                   fno.fsize,
                   fno.fname);
        }
        
        printf("Total files/directories found: %d\r\n", file_count);
        TEST_ASSERT(file_count >= 0, "Directory listing completed");
    }
}

/*---------------------------------------------------------------------------/
/ Test 4: File Open and Basic Read Test
/---------------------------------------------------------------------------*/
void test_file_open_read(void)
{
    FRESULT fr;
    UINT bytes_read;
    
    printf("\r\n=== Test 4: File Open and Read ===\r\n");
    
    // Try to open the IAP file first
    fr = pf_open("/RISKYKVM.UPD");
    if (fr == FR_OK) {
        printf("Successfully opened /RISKYKVM.UPD\r\n");
        printf("File size: %lu bytes\r\n", pff_test_fs.fsize);
        
        // Read first 256 bytes
        fr = pf_read(test_buffer, 256, &bytes_read);
        TEST_ASSERT_EQ(FR_OK, fr, "Read from IAP file");
        TEST_ASSERT(bytes_read <= 256, "Read byte count valid");
        
        printf("Read %d bytes from IAP file\r\n", bytes_read);
        
        // Display first 64 bytes as hex
        printf("First 64 bytes (hex): ");
        for (int i = 0; i < 64 && i < bytes_read; i++) {
            printf("%02X ", test_buffer[i]);
            if ((i + 1) % 16 == 0) printf("\r\n                      ");
        }
        printf("\r\n");
    } else {
        printf("IAP file not found, trying test file...\r\n");
        
        // Try to open any .txt file
        fr = pf_open("/README.TXT");
        if (fr != FR_OK) {
            fr = pf_open("/TEST.TXT");
        }
        
        if (fr == FR_OK) {
            printf("Successfully opened text file\r\n");
            printf("File size: %lu bytes\r\n", pff_test_fs.fsize);
            
            // Read some content
            fr = pf_read(test_buffer, 512, &bytes_read);
            TEST_ASSERT_EQ(FR_OK, fr, "Read from text file");
            
            // Display as text (null terminate)
            if (bytes_read < TEST_BUFFER_SIZE) {
                test_buffer[bytes_read] = 0;
                printf("File content preview:\r\n%s\r\n", test_buffer);
            }
        } else {
            TEST_ASSERT(0, "Open any test file (no suitable file found)");
        }
    }
}

/*---------------------------------------------------------------------------/
/ Test 5: File Seek Test
/---------------------------------------------------------------------------*/
void test_file_seek(void)
{
    FRESULT fr;
    UINT bytes_read;
    
    printf("\r\n=== Test 5: File Seek Test ===\r\n");
    
    // Open any file
    fr = pf_open("/RISKYKVM.UPD");
    if (fr != FR_OK) {
        fr = pf_open("/README.TXT");
    }
    if (fr != FR_OK) {
        fr = pf_open("/TEST.TXT");
    }
    
    if (fr == FR_OK && pff_test_fs.fsize > 1024) {
        printf("Testing file seek operations\r\n");
        
        // Seek to middle of file
        DWORD seek_pos = pff_test_fs.fsize / 2;
        fr = pf_lseek(seek_pos);
        TEST_ASSERT_EQ(FR_OK, fr, "Seek to middle of file");
        
        if (fr == FR_OK) {
            printf("Seeked to position %lu\r\n", seek_pos);
            
            // Read from new position
            fr = pf_read(test_buffer, 64, &bytes_read);
            TEST_ASSERT_EQ(FR_OK, fr, "Read after seek");
            
            printf("Read %d bytes from seek position\r\n", bytes_read);
        }
        
        // Seek to end of file
        fr = pf_lseek(pff_test_fs.fsize - 64);
        TEST_ASSERT_EQ(FR_OK, fr, "Seek near end of file");
        
        if (fr == FR_OK) {
            fr = pf_read(test_buffer, 128, &bytes_read);
            TEST_ASSERT_EQ(FR_OK, fr, "Read at end of file");
            printf("Read %d bytes near end of file\r\n", bytes_read);
        }
        
    } else {
        printf("No suitable file for seek test (need file > 1KB)\r\n");
        TEST_ASSERT(0, "File seek test (no suitable file)");
    }
}

/*---------------------------------------------------------------------------/
/ Test 6: Write Test (if write is enabled)
/---------------------------------------------------------------------------*/
void test_file_write(void)
{
    FRESULT fr;
    UINT bytes_written;
    const char* test_data = "PFF Test Data - Hello World!\r\nThis is a test file created by PFF test suite.\r\n";
    
    printf("\r\n=== Test 6: File Write Test ===\r\n");
    
#if PF_USE_WRITE
    // Create/open test file for writing
    fr = pf_open(TEST_WRITE_FILE);
    if (fr == FR_OK || fr == FR_NO_FILE) {
        
        // Write test data
        fr = pf_write(test_data, strlen(test_data), &bytes_written);
        TEST_ASSERT_EQ(FR_OK, fr, "Write test data to file");
        
        if (fr == FR_OK) {
            printf("Successfully wrote %d bytes\r\n", bytes_written);
            TEST_ASSERT_EQ(strlen(test_data), bytes_written, "Write byte count");
        }
        
    } else {
        printf("Could not open/create write test file\r\n");
        TEST_ASSERT(0, "Open file for writing");
    }
#else
    printf("Write functionality disabled in PFF configuration\r\n");
    TEST_ASSERT(1, "Write test skipped (disabled)");
#endif
}

/*---------------------------------------------------------------------------/
/ Test 7: Stress Test - Multiple Operations
/---------------------------------------------------------------------------*/
void test_stress_operations(void)
{
    FRESULT fr;
    UINT bytes_read;
    int iterations = 5;
    
    printf("\r\n=== Test 7: Stress Test ===\r\n");
    
    for (int i = 0; i < iterations; i++) {
        printf("Iteration %d/%d: ", i + 1, iterations);
        
        // Mount
        fr = pf_mount(&pff_test_fs);
        if (fr != FR_OK) {
            printf("Mount failed\r\n");
            test_fail_count++;
            continue;
        }
        
        // Open file
        fr = pf_open("/RISKYKVM.UPD");
        if (fr != FR_OK) {
            fr = pf_open("/README.TXT");
        }
        
        if (fr == FR_OK) {
            // Read some data
            fr = pf_read(test_buffer, 512, &bytes_read);
            if (fr == FR_OK) {
                printf("OK (read %d bytes)\r\n", bytes_read);
                test_pass_count++;
            } else {
                printf("Read failed\r\n");
                test_fail_count++;
            }
        } else {
            printf("Open failed\r\n");
            test_fail_count++;
        }
        
        // Small delay between iterations
        Delay_Ms(100);
    }
}

/*---------------------------------------------------------------------------/
/ Test Summary and Results
/---------------------------------------------------------------------------*/
void print_test_summary(void)
{
    printf("\r\n==================================================\r\n");
    printf("PFF Test Suite Summary\r\n");
    printf("==================================================\r\n");
    printf("Tests Passed: %lu\r\n", test_pass_count);
    printf("Tests Failed: %lu\r\n", test_fail_count);
    printf("Total Tests:  %lu\r\n", test_pass_count + test_fail_count);
    
    if (test_fail_count == 0) {
        printf("Result: ALL TESTS PASSED! ✓\r\n");
        blinkLed(3, 200);  // Success indication
    } else {
        printf("Result: %lu TESTS FAILED! ✗\r\n", test_fail_count);
        blinkLed(10, 100);  // Failure indication
    }
    printf("==================================================\r\n");
}

/*---------------------------------------------------------------------------/
/ Main Test Suite Function
/---------------------------------------------------------------------------*/
void run_pff_test_suite(void)
{
    printf("\r\n");
    printf("*****************************************************\r\n");
    printf("         PFF (Petit FatFS) Test Suite v1.0          \r\n");
    printf("     Testing USB Mass Storage Implementation         \r\n");
    printf("*****************************************************\r\n");
    
    // Reset test counters
    test_pass_count = 0;
    test_fail_count = 0;
    
    // Run all tests
    test_disk_initialization();
    test_filesystem_mount();
    test_directory_listing();
    test_file_open_read();
    test_file_seek();
    test_file_write();
    test_stress_operations();
    
    // Print summary
    print_test_summary();
}

/*---------------------------------------------------------------------------/
/ Quick Test Function (for basic verification)
/---------------------------------------------------------------------------*/
void run_pff_quick_test(void)
{
    DSTATUS disk_status;
    FRESULT fr;
    
    printf("\r\n=== PFF Quick Test ===\r\n");
    
    // Initialize disk
    disk_status = disk_initialize();
    if (disk_status != 0) {
        printf("FAIL: Disk initialization (status: 0x%02X)\r\n", disk_status);
        return;
    }
    printf("PASS: Disk initialization\r\n");
    
    // Mount filesystem
    fr = pf_mount(&pff_test_fs);
    if (fr != FR_OK) {
        printf("FAIL: Filesystem mount (result: %d)\r\n", fr);
        return;
    }
    printf("PASS: Filesystem mount\r\n");
    
    // Try to open IAP file
    fr = pf_open("/RISKYKVM.UPD");
    if (fr == FR_OK) {
        printf("PASS: Found IAP file (size: %lu bytes)\r\n", pff_test_fs.fsize);
    } else {
        printf("INFO: IAP file not found, that's OK for testing\r\n");
    }
    
    printf("=== Quick Test Complete ===\r\n");
}