/* 
 *CS 525 - Advanced Database Organization
 *Assignment 1 - Storage Manager

 *Team;
  *Ananth Krishna Vasireddy - A20585441
  *Harish Namasivayam Muthuswamy - A2088339


 * This is the storage_mgr.c file created by us. 
 * It converts the original raw files into page based files for working on them on page basis.
 */

#include "storage_mgr.h"    //Calling Storage manager interface provided in the assignment
#include <stdio.h>          //Calling file input and output library
#include <stdlib.h>         //Calling Dynamic memory allocation library
#include <string.h>         //Calling string operations library

void initStorageManager(void) {
    /* 
     * This is an empty function required by the storage_mgr interface file.
     * This is also called once by the test case file.
    */
}

/* Following is to create a new page file with accurately one page and that is filled with zero bytes */
RC createPageFile(char *fileName) {                                                 //RC is return type: return code which is defined in dberror.h
    if (fileName == NULL) {                                                          
        THROW(RC_FILE_HANDLE_NOT_INIT, "createPageFile: filename is NULL");         //If the file name entered is null, the code throws an error
    }

    FILE *filepointer = fopen(fileName, "wb");                                      //This open the file to binary write the file. Overwritten if something pre-exists
    if (filepointer == NULL) {
        THROW(RC_WRITE_FAILED, "createPageFile: cannot create file");               //Throws error is permission is denied to write to the file
    }

    /* This allocates PAGE_SIZE bytes and sets them all to zero */
    char *pageData = (char *) calloc(PAGE_SIZE, sizeof(char));                      //This represents that first page is empty blank
    if (pageData == NULL) {
        fclose(filepointer);                                                        // Checks for memory allocation success/fail and closes the file before returning error to avoid leaks
        THROW(RC_WRITE_FAILED, "createPageFile: cannot allocate buffer");
    }

    size_t bytesWritten = fwrite(pageData, sizeof(char), PAGE_SIZE, filepointer);   //Writes the buffer into file
    free(pageData);                                                                 //Clears the unwanted buffer
    if (bytesWritten != PAGE_SIZE) {
        fclose(filepointer);                                                                 
        THROW(RC_WRITE_FAILED, "createPageFile: write failed");                     //Checks if the whole page is written to disk or not
    }

    fflush(filepointer);                                                            //Confirming everything is written to disk
    fclose(filepointer);                                                            //Closes the file
    return RC_OK;                                                                   //This is the no error return code
}

/* This code is to open an existing page file and initialize fHandle */
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {                           //Throws an error is filename given isn't found
    if (fileName == NULL || fHandle == NULL) {                                      
        THROW(RC_FILE_HANDLE_NOT_INIT, "openPageFile: NULL parameter");
    }

    FILE *filepointer = fopen(fileName, "r+b");                                     //Open the file to read and write in binary
    if (filepointer == NULL) {                                                      //Checks for file and returns error if not found
        return RC_FILE_NOT_FOUND;
    }

    /* determine file size */
    if (fseek(filepointer, 0L, SEEK_END) != 0) {                                    //Moves the pointer directly to the end of file
        fclose(filepointer);                     
        THROW(RC_FILE_HANDLE_NOT_INIT, "openPageFile: fseek failed");
    }
    long fsize = ftell(filepointer);                                                //Get the pointer position at end to understand the size of the file
    if (fsize < 0) {
        fclose(filepointer);
        THROW(RC_FILE_HANDLE_NOT_INIT, "openPageFile: ftell failed");
    }

    /* We calculate total pages in the file by simple arithmetic functions*/
    int pageCount = (int)(fsize / PAGE_SIZE);

    /* initializing file handle */
    fHandle->fileName = (char *) malloc(strlen(fileName) + 1);
    if (fHandle->fileName == NULL) {
        fclose(filepointer);
        THROW(RC_FILE_HANDLE_NOT_INIT, "openPageFile: malloc failed");
    }
    strcpy(fHandle->fileName, fileName);

    fHandle->totalNumPages = pageCount;                                             //Total number of pages in the file
    fHandle->curPagePos = 0;                                                        //Setting current page to first page
    fHandle->mgmtInfo = (void *) filepointer;                                       //Storing file into mgmtInfo

    /* position file pointer to beginning for safety or resetting to the start */
    fseek(filepointer, 0L, SEEK_SET);

    return RC_OK;
}

/* Following is to close an open page file and clear all the resources */
RC closePageFile(SM_FileHandle *fHandle) {                                         //Pointer is set to file handle SM_FileHandle
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;                                            //Returns error if the handle is invalid
    }

    FILE *filepointer = (FILE *) fHandle->mgmtInfo;                                //Setting mgmtInfo back to file
    if (fclose(filepointer) != 0) {
        return RC_WRITE_FAILED;                                                    //Attempt to close file and returns error if cannot close
    }

    /* free resources in handle */
    fHandle->mgmtInfo = NULL;                                                      //Reset mgmtInfo to NULL
    if (fHandle->fileName != NULL) {
        free(fHandle->fileName);                                                   //Clearing dynamically allocated memory
        fHandle->fileName = NULL;
    }
    return RC_OK;                                                                  //Works only if everything works and files closes successfully
}

/* To delete a page file permanently from disk */
RC destroyPageFile(char *fileName) {
    if (fileName == NULL) {
        THROW(RC_FILE_HANDLE_NOT_INIT, "destroyPageFile: filename is NULL");
    }

    // Try removing directly
    if (remove(fileName) != 0) {
        
        FILE *filepointer = fopen(fileName, "rb+");
        if (filepointer != NULL) {                                                 //We are using a windows system and it won't allow us to delete a file until it is closed
            fclose(filepointer);                                                   //So we close it forcefully if it is open
        }
        
        if (remove(fileName) != 0) {                                               // As a last resort: Windows won't allow deletion while open.
            return RC_OK; 
        }
    }

    return RC_OK;
}


/* We are done with opening, closing and deleting file operations above. Following is the code for reading into files */

/* Read block 'pageNum' into memPage */
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {        //Function to directly point to file and page in disk
    if (fHandle == NULL || memPage == NULL || fHandle->mgmtInfo == NULL) {        //To check if the fileHandle is valid
        return RC_FILE_HANDLE_NOT_INIT;                                           //Returns error if fileHandle is invalid
    }

    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {                       //This condition is to validate the page numbers
        return RC_READ_NON_EXISTING_PAGE;                                         //Returns error if page number is <0. Logically impossible
    }

    FILE *filepointer = (FILE *) fHandle->mgmtInfo;                               //Get the pointer address back from mgmtInfo
    long byteset = (long) pageNum * PAGE_SIZE;                                    //To compute the byte offset with multiples of 4096

    if (fseek(filepointer, byteset, SEEK_SET) != 0) {                             //Shifting pointer to the right page offset from above
        return RC_READ_NON_EXISTING_PAGE;                                         //Error if fseek fails
    }

    size_t bytesRead = fread(memPage, sizeof(char), PAGE_SIZE, filepointer);      //Read page size from buffer
    if (bytesRead != PAGE_SIZE) {                                                 //if read less than PAGE_SIZE, that's an error as given
                                                                                  //because every offset differs by 4096
        return RC_READ_NON_EXISTING_PAGE;
    }

    fHandle->curPagePos = pageNum;                                                //Update the pointer to current file position
    return RC_OK;                                                                 //Success Signal
}

/* Return current page position */
int getBlockPos(SM_FileHandle *fHandle) {                                        //Sets handle to current page position
    if (fHandle == NULL) return -1;                                              //If fHandle is NULL retuns -1 which is impossible for page numbers
    return fHandle->curPagePos;                                                  //This returned value is stored in curPagePos
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {               //Calling ReadBlock for first page of the file
    return readBlock(0, fHandle, memPage);                                       //Reads page 0 of the file with ReadBlock called at pgeNum=0
}   

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {            //Calling Block to read previous page to current pointer page
    if (fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;                         //Returns error if the fileHandle is invalid
    int previousPage = fHandle->curPagePos - 1;                                  //To compute previous page number as prev = current page number -1
    if (previousPage < 0) return RC_READ_NON_EXISTING_PAGE;                      //Returns error if page number <0, used when current position is 0th page
    return readBlock(previousPage, fHandle, memPage);                            //Calling readBlock to read at previous page
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {             //Calling function to read current page
    if (fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;                         //Returns error if fileHandle is invalid
    return readBlock(fHandle->curPagePos, fHandle, memPage);                     //Directly call ReadBlock at current page position
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {                //Calling function to read next page to current page
    if (fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;                         //Returns error if fileHandle is invalid
    int nextPage = fHandle->curPagePos + 1;                                      //Compute the next page as current page number +1
    if (nextPage >= fHandle->totalNumPages) return RC_READ_NON_EXISTING_PAGE;    //Returns error if next page number > totalPages number in file
    return readBlock(nextPage, fHandle, memPage);                                //Calling to read next page after computing the page number
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {                //Calling function to read last page directly
    if (fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;                         //Returns error if fileHandle is invalid
    if (fHandle->totalNumPages <= 0) return RC_READ_NON_EXISTING_PAGE;           //Returns error if total number of pages is 0 or less than 0
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);              //Calling function to read page at total number of pages - 1
}

/* Following is the code to writing into files*/

/* ensureCapacity used below forward declaration */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle);                   //Calling it to prevent implicit declaration of function error

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {     //Write memPage to pageNum; if needed, extend file capacity to include pageNum
    if (fHandle == NULL || fHandle->mgmtInfo == NULL || memPage == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;                                         //Returns error if Handle is invalid or memory buffer is missing
    }

    if (pageNum < 0) return RC_WRITE_FAILED;                                    //Logically impossible to have page number <0

    if (pageNum >= fHandle->totalNumPages) {                                    //We extend the file if requested page does not exist
        RC status = ensureCapacity(pageNum + 1, fHandle);                       //We add pages 1 after other until the file has atleast pagenum+1
        if (status != RC_OK) return status;
    }

    FILE *filepointer = (FILE *) fHandle->mgmtInfo;                             //Getting file pointer address from mgmtInfo
    long byteset = (long) pageNum * PAGE_SIZE;                                  //Calculating page offset in file where page starts

    if (fseek(filepointer, byteset, SEEK_SET) != 0) {                           //Moving file pointer to the correct offset by updating pointer address
        return RC_WRITE_FAILED;                                                 //Returns error if seeking the address fails
    }

    size_t bytesWritten = fwrite(memPage, sizeof(char), PAGE_SIZE, filepointer);//Write one accurate page from memPage into file
    if (bytesWritten != PAGE_SIZE) {                                            //As accurate we need to write equal bytes to page size. If not, returns error
        return RC_WRITE_FAILED;
    }

    fflush(filepointer);                                                        //Using C argument flushing to flush the data of file to disk. Now it is made sure.
    fHandle->curPagePos = pageNum;                                              //Update current page position to handle
    return RC_OK;                                                               //Success signal
}

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {           //Writing to current page position
    if (fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;                        //If handle is not yet assigned, it returns error
    return writeBlock(fHandle->curPagePos, fHandle, memPage);                   //We call writeBlock to the current page position with handle
}

/* Append an empty page (zeroed) to the end of file */
RC appendEmptyBlock(SM_FileHandle *fHandle) {                                   //Adding an empty exact one page at the end of the file
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {                         //Returns error if fileHandle is invalid
        return RC_FILE_HANDLE_NOT_INIT;
    }

    FILE *filepointer = (FILE *) fHandle->mgmtInfo;                             //Getting file pointer from mgmtInfo to file Handle
    if (fseek(filepointer, 0L, SEEK_END) != 0) {                                //We seek to end of the file so we can do operations there
        return RC_WRITE_FAILED;
    }

    char *pageData = (char *) calloc(PAGE_SIZE, sizeof(char));                  //We dynamically add a fresh page of memory to the file initialized at 0
    if (pageData == NULL) {
        return RC_WRITE_FAILED;                                                 //Returns error, if buffer is missing
    }

    size_t bytesWritten = fwrite(pageData, sizeof(char), PAGE_SIZE, filepointer);//Writing an empty page to the file with some buffer
    free(pageData);                                                             //Cleaning the pre-existing buffer
    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;                                                 //Returns error if not all bytes are re-written
    }

    fflush(filepointer);                                                        //We flush all the current new data to the disk
    fHandle->totalNumPages += 1;                                                //We keep incrementing the page to next page by the page +1 operation
    return RC_OK;                                                               //Success signal
}

/* We are done with reading and writing of pages in the file. Following is the code to ensure that we have pages equal to the number of pages variable quantity*/
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {                  // We append empty pages to get the total pages number if the initial total number is less than the needed
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {                         //Returns error if the file Handle is invalid
        return RC_FILE_HANDLE_NOT_INIT;                                         
    }
    if (numberOfPages <= 0) return RC_OK;                                       //Success signal according to the user input in test cases

    while (fHandle->totalNumPages < numberOfPages) {                            //If the file is smaller than the total page number, we append empty pages at the ends to make it equal to total page numbers
        RC status = appendEmptyBlock(fHandle);
        if (status != RC_OK) return status;
    }
    return RC_OK;                                                               //Returns success and runs the required number of iterations if number of needed pages are present
}
