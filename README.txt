__________________***_________________
CS525 - Advanced Database Organization
Assignment 1 - Storage Manager
__________________***_________________


By: Ananth Krishna Vasireddy (A20585441) & Harish Namashivaya (A2088339)
Submission Date  : 09/07/2025

------------------------------------------------------------
Description
------------------------------------------------------------
This project implements a simple Storage Manager in C. This Storage Manager is responsible for:
 - Creating, opening, closing, and destroying page files.
 - Reading and writing fixed-size pages (PAGE_SIZE = 4096).
 - Managing current page position.
 - Extending files dynamically with empty pages.

All operations follow the interface defined in "storage_mgr.h". This "storage_mgr.h" is provided in the assignment.

------------------------------------------------------------
2. All Files and their description in this Folder
------------------------------------------------------------

Provided files (Didn't modify, kept them the same):
 - dberror.c / dberror.h   : Error handling and constants.
 - storage_mgr.h           : API definition for Storage Manager.
 - test_assign1_1.c        : Instructor-provided test cases.
 - test_helper.h           : Macros for testing.

Implemented by us:
 - storage_mgr.c           : Implementation of Storage Manager API.
 - test_assign1_extra.c    : Additional test cases for full coverage.
 - README.txt              : Project documentation and instructions to run.
 - Makefile                : Link between the storage_mgr.c and test cases files.

------------------------------------------------------------
3. Building on Windows (without make -  we didn't use make as it needs excess files to be installed and we didn't want to make any more changes to our system as it might change the working of several other items or softwares)
------------------------------------------------------------

We use GCC for this project and it has to be installed from MinGW-w64 (We used a windows OS. So, we used this.)
Steps for running project using GCC;
(1) Download GCC and MinGW-w64 from "https://winlibs.com/"
(2) Install it at "C:\Program Files\mingw-w64\"
(3) Add bin folder to your PATH (so gcc can be called in cmd). For this, go to Windows -> System Properties -> Advanced -> Environment Variables -> add "C:\Program Files\mingw-w64\mingw-w64\bin" by editing the path.
***Note: This is how we did it and have included the settings in the video end as well. Check that so if any changes are there in your installations, you can do it accordingly.

Now;
This project uses GCC directly from the command line. 
No 'make' tool is required.

Open Command Prompt in the `assign1` folder. Run it as administrator as it uses all the softwares required without any excess changes in settings.

(1) Compile the provided tests:

    gcc -Wall -g -o test_assign1 test_assign1_1.c dberror.c storage_mgr.c

(2) Compile the extra tests:

    gcc -Wall -g -o test_assign1_extra test_assign1_extra.c dberror.c storage_mgr.c

------------------------------------------------------------
4. Running the Tests
------------------------------------------------------------

Run the executables after compilation:

    test_assign1.exe
    test_assign1_extra.exe

Both will create and delete temporary files named
`test_pagefile.bin` or `test_pagefile_extra.bin`.

------------------------------------------------------------
5. What the tests do?
------------------------------------------------------------

test_assign1 (provided):
 - Creating and destroying a page file.
 - Opening and closing a file.
 - Writing and reading a single page.

test_assign1_extra (added for full coverage):
 - appendEmptyBlock: adds new empty pages.
 - ensureCapacity  : ensures file has a minimum size.
 - Relative reads  : readFirstBlock, readNextBlock, readPreviousBlock,
                     readCurrentBlock, readLastBlock.
 - writeBlock      : writes data at an absolute position.
 - writeCurrentBlock: writes at the current position.
 - Error handling  : 
     * opening a non-existing file
     * reading before the first page
     * reading after the last page

------------------------------------------------------------
6. Cleaning Up
------------------------------------------------------------

After running, if you see any leftover test files 
(like test_pagefile.bin or test_pagefile_extra.bin), 
you can safely delete them:

    del test_pagefile*.bin

------------------------------------------------------------
7. Notes
------------------------------------------------------------

 - mgmtInfo stores a FILE* pointer for file I/O.
 - Each file is created with 1 page (4096 bytes of '\0').
 - readBlock and writeBlock always operate on PAGE_SIZE bytes.
 - writeBlock automatically extends the file using ensureCapacity
   if the target page number is beyond current size.
 - Error codes are consistent with dberror.h (e.g.,
   RC_FILE_NOT_FOUND, RC_READ_NON_EXISTING_PAGE).

------------------------------------------------------------
This is the end of README. We have provided all the instruction to run the codes and project.
We have also attached the screenshots of the outputs in this repository
============================================================

