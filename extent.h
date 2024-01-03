/**************************************************************

* Project: Basic File System
* File: extent.h
*
* Description: Interface of Extent functions
*
**************************************************************/

#ifndef _EXTENT_H
#define _EXTENT_H

#define STARTEXTENT 6400
#define LOCEXTENT 3

typedef struct extent {
	int blockNumber;		// Location of starting block
	int blockCount;			// Number of blocks starting from blockNumber
} extent;

// Copy extent values from source to destination
int extentCopy(extent dest, extent source);

#endif