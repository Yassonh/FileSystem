/**************************************************************
 * Project: Basic File System
 *
 * File: vcn.c
 *
 * Description: Setting up Our VCB
 *
 ***********************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vcb.h"
#include "freespace.h"
#include "fsLow.h"

VCB *vcb;
global *g;

int vcbInit(int blockSize, int blockCount)
{

	// Malloc a Block of memory as VCB pointer and LBAread block 0
	vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);

	// Malloc memory for global struct to use its data for the file system
	g = malloc(sizeof(global));

	// Replaced existing string literal definition of g->curPath with this dynamic one
	// Malloc for g->curPath and initialize it to "/"
	g->curPath = malloc(strlen("/") + 1); // +1 is for the null terminator
	if (g->curPath == NULL)
	{
		// Debug message for malloc here
		printf("Malloc failed for g->curPath inn vcb.c\n");
		return -1;
	}
	strcpy(g->curPath, "/"); // Copy in the "/"

	// If signature does not match, format the volume
	if (vcb->signature != VCBSIGNATURE)
	{

		vcb->signature = VCBSIGNATURE;
		vcb->blockSize = blockSize;
		vcb->blockCount = blockCount;

		// Init free space map
		freeSpaceInit();
		// Init root directory
		g->loadedDir = dirInit(STARTDE, NULL);

		// Write VCB to disk at block[0]
		LBAwrite(vcb, 1, 0);
	}

	return 0;
}