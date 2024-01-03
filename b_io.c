/**************************************************************
 * File: b_io.c
 *
 * Description: Basic File System - Key File I/O Operations
 *
 **************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	/** TODO add all the information you need in the file control block **/
	char *buf;	// holds the open file buffer
	int index;	// holds the current position in the buffer
	int buflen; // holds how many valid bytes are in the buffer
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
	//printf("Inside b_init\n");
	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; // indicates a free fcbArray
	}

	startup = 1;
	//printf("b_init done\n");
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
	//printf("Start of getFCB\n");
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			//printf("FCB found at %d\n", i);
			return i; // Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); // all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
// Interface to open a buffered file
// Interface to open a buffered file
b_io_fd b_open(char *filename, int flags)
{
    //printf("Start of b_open\n");
    b_io_fd returnFd;

    if (startup == 0)
        b_init(); // Initialize our system

    returnFd = b_getFCB(); // get our own file descriptor
                           // check for error - all used FCB's

    if (returnFd == -1)
    {
        printf("bOpen reports: Unable to get a file control block\n");
        return -1;
    }

    // Open the file using the existing open command with the function parameters
    int fileDesc = open(filename, flags);
    if (fileDesc == -1)
    {
        perror("bOpen reports: Open command was unable to open the file");
        return -1;
    }

    //printf("Opened file %s with file descriptor: %d\n", filename, fileDesc);

    // Malloc some buffer space for the file control block array
    fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
    if (fcbArray[returnFd].buf == NULL)
    {
        printf("bOpen reports: fcbArray malloc failed\n");
        close(fileDesc);
        return -1;
    }

    // Set values for file control block fields
    fcbArray[returnFd].index = 0;
    fcbArray[returnFd].buflen = 0;

    //printf("End of b_open, and it's returning file descriptor: %d\n", returnFd);
    return fileDesc; // return the file descriptor obtained from b_open
}




// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	printf("Start of b_seek\n");
	// Partially based on description given at https://man7.org/linux/man-pages/man2/lseek.2.html
	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		printf("bSeek reports invalid filed descriptor, file descriptor is %d\n", fd);
		perror("bseek failed");
		return (-1); // invalid file descriptor
	}

	// Print parameters to see if any of them are wrong
    printf("Before lseek: File descriptor = %d, Offset = %ld, Whence = %d\n", fd, offset, whence);

	// Seek using existing linux lseek function as mentioned on the manpage
	off_t calcNewPosition = lseek(fd, offset, whence);
	if (calcNewPosition == -1)
	{
		perror("Before lseek: ");
		printf("bSeek reports: calcNewPosition failed\n");
		return -1;
	}

	printf("End of b_seek\n");
	return 0;
}

// Write to file using the existing linux write function described at https://man7.org/linux/man-pages/man2/write.2.html
int b_write(b_io_fd fd, char *buffer, int count)
{
	printf("Start of b_write\n");
	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	
	ssize_t bytesWritten = write(fd, buffer, count);
	if (bytesWritten == -1)
	{
		printf("bWrite reports: write function failed\n");
		return -1;
	}

	printf("End of b_write\n");
	return bytesWritten;
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+

// Read from file using the existing read command described at https://man7.org/linux/man-pages/man2/read.2.html

int b_read(b_io_fd fd, char *buffer, int count)
{
	//printf("Start of b_read\n");

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		printf("b_read reports: invalid file descriptor\n");
		return (-1); // invalid file descriptor
	}


	//printf("bRead trying read command now\n");

	// Read from file using the existing read command described at https://man7.org/linux/man-pages/man2/read.2.html
	ssize_t bytesRead = read(fd, buffer, count);
    if (bytesRead == -1)
    {
        perror("bRead reports: read function failed to read from the file");
        return -1;
    }
    else if (bytesRead == 0)
    {
		printf("\n");
        //printf("\nbRead reports: End of file reached!\n");
    }
    else
    {
        //printf("bRead reports: Successfully read %zd bytes\n", bytesRead);
    }

    //printf("End of b_read\n");
    return bytesRead;
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
	//printf("Start of b_close\n");
	if (startup == 0)
	{
		b_init();
	}

	// Make sure that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // Invalid file descriptor
	}

	// Close fd (file descriptor) and free the buffer so things don't segmentation fault
	// Closed using existing close function described at https://man7.org/linux/man-pages/man2/close.2.html
	close(fd);
	free(fcbArray[fd].buf);
	// Mark file control block as free space
	fcbArray[fd].buf = NULL;
	//printf("End of b_close\n");
	return 0;
}
