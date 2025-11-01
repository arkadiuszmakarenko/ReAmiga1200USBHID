/*---------------------------------------------------------------------------/
/  PFF Library Test Suite Header
/  Tests USB Mass Storage functionality with Petit FatFS
/---------------------------------------------------------------------------*/

#ifndef PFF_TEST_H
#define PFF_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------/
/ Function Prototypes
/---------------------------------------------------------------------------*/

/* Main test suite function - runs all tests */
void run_pff_test_suite(void);

/* Quick test function - basic verification */
void run_pff_quick_test(void);

/* Individual test functions */
void test_disk_initialization(void);
void test_filesystem_mount(void);
void test_directory_listing(void);
void test_file_open_read(void);
void test_file_seek(void);
void test_file_write(void);
void test_stress_operations(void);

/* Test utilities */
void print_test_summary(void);

#ifdef __cplusplus
}
#endif

#endif /* PFF_TEST_H */