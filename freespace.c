/**************************************************************
* Project: Basic File System
* File: freespace.c
*
* Description: Basic File System - Free space functions
*
**************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "freespace.h"
#include "vcb.h"
#include "fsLow.h"
#include "util.h"

int freeSpaceInit() {
    // Find bytes needed to malloc and blocksNeeded
    int bytesNeeded = STARTEXTENT * sizeof(extent);
    int blocksNeeded = getBlocksPerByte(bytesNeeded, vcb->blockSize);
    int bytesUsed = blocksNeeded * vcb->blockSize;

    // Store free space map starting at Block 1
    vcb->freeSpaceMapLoc.blockNumber = 1;
    vcb->freeSpaceMapLoc.blockCount = blocksNeeded;

    g->freeSpaceMap = malloc(bytesUsed);
    g->freeSpaceExtentCount = blocksNeeded / sizeof(extent);

    // Set default values to free space map
    for (int i = 1; i < g->freeSpaceExtentCount; i++) {
        g->freeSpaceMap[i].blockNumber = 0;
        g->freeSpaceMap[i].blockCount = 0;
    }

    // Mark VCB and free space map as used [0 to totalBlocks - blocksNeeded - 1]
    g->freeSpaceMap[0].blockNumber = blocksNeeded + 1;
    g->freeSpaceMap[0].blockCount = vcb->blockCount - blocksNeeded - 1;

    // Write free space map to disk starting from Block 1
    LBAwrite(g->freeSpaceMap, blocksNeeded, 1);

    
    return 0;
}

extent freeSpaceAlloc(int blockCount) {
    extent location;
    for (int i = 0; i < g->freeSpaceExtentCount; i++) {
        if (g->freeSpaceMap[i].blockCount >= blockCount) {
            int end = g->freeSpaceMap[i].blockNumber + g->freeSpaceMap[i].blockCount;
            int overlap = 0;

            for (int j = 0; j < g->freeSpaceExtentCount; j++) {
                if (j != i && g->freeSpaceMap[j].blockNumber < end && 
                    g->freeSpaceMap[j].blockNumber + g->freeSpaceMap[j].blockCount > g->freeSpaceMap[i].blockNumber) {
                    overlap = 1;
                    break;
                }
            }

            if (!overlap) {
                location.blockNumber = g->freeSpaceMap[i].blockNumber;
                location.blockCount = blockCount;
                g->freeSpaceMap[i].blockNumber += blockCount;
                g->freeSpaceMap[i].blockCount -= blockCount;

                // Write to disk updated free space map
                LBAwrite(g->freeSpaceMap, vcb->freeSpaceMapLoc.blockCount, vcb->freeSpaceMapLoc.blockNumber);

                return location;
            }
        }
    }
    
    // No free space found
    location.blockNumber = 0;
    location.blockCount = 0;
    return location;
}
