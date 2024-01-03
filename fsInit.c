/**************************************************************
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	// This is needed to start the file system, without it, nothing happens.
	vcbInit(blockSize, numberOfBlocks);
	dirInit(2, NULL);

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}