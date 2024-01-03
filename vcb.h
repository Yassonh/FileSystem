/**************************************************************
* Project: Basic File System
*
* File: dir.c
*
* Description: VCB and global Structures 
*
***********************************************************************************************/

#ifndef _VCB_H
#define _VCB_H

#include "extent.h"
#include "dir.h"

#define VCBSIGNATURE 1234

// Keep track of these variables globally for file system
typedef struct global {
  extent *freeSpaceMap;
  int freeSpaceExtentCount;
  directoryEntry *loadedDir;
  directoryEntry *rootDir;
  int actualDECount;
  char *curPath;
} global;

typedef struct VCB {
	int signature;                    // Name/ID for VCB. Value is "VCBSIGNATURE"
  int blockSize;                    // Size of blocks in bytes in volume
  int blockCount;                   // Number of blocks in volume
  extent freeSpaceMapLoc;           // Free space map location
	directoryEntry rootDir;           // Root Directory Entry
} VCB;

// Global VCB variable is declared in vcb.c
extern VCB *vcb;
extern global *g;

int vcbInit(int blockSize, int blockCount);

#endif