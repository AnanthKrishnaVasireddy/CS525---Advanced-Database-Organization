#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

char *testName;

#define TESTPF "test_pagefile_extra.bin"

/* prototypes */
static void testAppendAndEnsure(void);
static void testRelativeReads(void);
static void testErrorCases(void);
static void testWriteBlocks(void);

int main(void) {
    testName = "";
    initStorageManager();

    testAppendAndEnsure();
    testRelativeReads();
    testErrorCases();
    testWriteBlocks();

    return 0;
}

/* ===========================================================
   Test appending blocks and ensuring capacity
   =========================================================== */
void testAppendAndEnsure(void) {
    SM_FileHandle fh;

    testName = "test append empty blocks and ensure capacity";

    /* create and open */
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    ASSERT_EQUALS_INT(1, fh.totalNumPages, "new file has one page");

    /* append 2 empty pages */
    TEST_CHECK(appendEmptyBlock(&fh));
    TEST_CHECK(appendEmptyBlock(&fh));

    ASSERT_EQUALS_INT(3, fh.totalNumPages, "file should now have 3 pages");

    /* ensure capacity of 5 pages */
    TEST_CHECK(ensureCapacity(5, &fh));
    ASSERT_EQUALS_INT(5, fh.totalNumPages, "file should now have 5 pages");

    /* cleanup */
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));

    TEST_DONE();
}

/* ===========================================================
   Test relative reading methods
   =========================================================== */
void testRelativeReads(void) {
    SM_FileHandle fh;
    SM_PageHandle ph;
    int i;

    testName = "test relative block reading";

    ph = (SM_PageHandle) malloc(PAGE_SIZE);

    /* create file with 3 pages */
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));
    TEST_CHECK(appendEmptyBlock(&fh));
    TEST_CHECK(appendEmptyBlock(&fh));

    ASSERT_EQUALS_INT(3, fh.totalNumPages, "file should now have 3 pages");

    /* fill each page with a character */
    for (i = 0; i < PAGE_SIZE; i++) ph[i] = 'A';
    TEST_CHECK(writeBlock(0, &fh, ph));

    for (i = 0; i < PAGE_SIZE; i++) ph[i] = 'B';
    TEST_CHECK(writeBlock(1, &fh, ph));

    for (i = 0; i < PAGE_SIZE; i++) ph[i] = 'C';
    TEST_CHECK(writeBlock(2, &fh, ph));

    /* test relative reads */
    TEST_CHECK(readFirstBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'A', "first page has 'A'");

    TEST_CHECK(readNextBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'B', "second page has 'B'");

    TEST_CHECK(readNextBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'C', "third page has 'C'");

    /* now try previous block */
    TEST_CHECK(readPreviousBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'B', "previous page is 'B'");

    /* current block */
    TEST_CHECK(readCurrentBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'B', "current page is still 'B'");

    /* last block */
    TEST_CHECK(readLastBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'C', "last page is 'C'");

    /* cleanup */
    free(ph);
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));

    TEST_DONE();
}

/* ===========================================================
   Test error cases for robustness
   =========================================================== */
void testErrorCases(void) {
    SM_FileHandle fh;
    SM_PageHandle ph;

    testName = "test error cases";

    ph = (SM_PageHandle) malloc(PAGE_SIZE);

    /* Opening a non-existing file */
    ASSERT_ERROR(openPageFile("does_not_exist.bin", &fh), "opening missing file should fail");

    /* Create a new file with 1 page */
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    /* Try to read previous block at beginning (curPagePos=0) */
    TEST_CHECK(readFirstBlock(&fh, ph));
    ASSERT_ERROR(readPreviousBlock(&fh, ph), "cannot read before first page");

    /* Move to last block (page 0, since only one page) */
    TEST_CHECK(readLastBlock(&fh, ph));
    ASSERT_ERROR(readNextBlock(&fh, ph), "cannot read beyond last page");

    /* cleanup */
    free(ph);
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));

    TEST_DONE();
}

/* ===========================================================
   Test writing with writeBlock and writeCurrentBlock
   =========================================================== */
void testWriteBlocks(void) {
    SM_FileHandle fh;
    SM_PageHandle ph;
    int i;

    testName = "test writing blocks";

    ph = (SM_PageHandle) malloc(PAGE_SIZE);

    /* create and open */
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    /* fill buffer with 'X' and write to first block */
    for (i = 0; i < PAGE_SIZE; i++) ph[i] = 'X';
    TEST_CHECK(writeBlock(0, &fh, ph));

    /* read back */
    TEST_CHECK(readFirstBlock(&fh, ph));
    ASSERT_TRUE(ph[0] == 'X', "first page contains 'X'");

    /* append a new page */
    TEST_CHECK(appendEmptyBlock(&fh));

    /* fill buffer with 'Y' and write to current block (page 1) */
    for (i = 0; i < PAGE_SIZE; i++) ph[i] = 'Y';
    fh.curPagePos = 1;  /* set to page 1 */
    TEST_CHECK(writeCurrentBlock(&fh, ph));

    /* read back page 1 */
    TEST_CHECK(readBlock(1, &fh, ph));
    ASSERT_TRUE(ph[0] == 'Y', "second page contains 'Y'");

    /* cleanup */
    free(ph);
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));

    TEST_DONE();
}
