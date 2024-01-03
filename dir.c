/**************************************************************
 *
 * Description: Root Directory initialization
 *
 **************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vcb.h"
#include "freespace.h"
#include "util.h"
#include "fsLow.h"

directoryEntry *dirInit(int baseNumDE, directoryEntry *parent)
{

	// Optimize bytes usage in block to avoid wasted bytes
	int bytesNeeded = baseNumDE * sizeof(directoryEntry);
	int blocksNeeded = getBlocksPerByte(bytesNeeded, vcb->blockSize);
	int bytesUsed = blocksNeeded * vcb->blockSize;
	g->actualDECount = bytesUsed / sizeof(directoryEntry);
	printf("Actual Directory Entry count: %d\n", g->actualDECount);
	bytesNeeded = g->actualDECount * sizeof(directoryEntry);

	// Free later by caller
	directoryEntry *dir = malloc(bytesUsed);

	// Alloc free space
	extent location = freeSpaceAlloc(blocksNeeded);

	// Mark all DE starting from dir[2] as unused
	for (int i = 2; i < g->actualDECount; i++)
	{
		dir[i].name[0] = '\0';
	}

	// Set first dir to "."
	strcpy(dir[0].name, ".");
	extentCopy(dir[0].location, location);
	dir[0].size = bytesNeeded;
	dir[0].isDirectory = 1;
	dir[0].timeCreate = time(NULL);
	dir[0].timeMod = dir[0].timeCreate;
	dir[0].timeAccess = dir[0].timeCreate;

	// If doing Root Directory, then copy dir[0] except name
	if (parent == NULL)
	{
		strcpy(dir[1].name, "..");
		extentCopy(dir[1].location, location);
		dir[1].size = bytesNeeded;
		dir[1].isDirectory = 1;
		dir[1].timeCreate = time(NULL);
		dir[1].timeMod = dir[1].timeCreate;
		dir[1].timeAccess = dir[1].timeCreate;

		// Store info about root into vcb
		vcb->rootDir = dir[0];

		// Load into memory for further use
		g->rootDir = malloc(bytesUsed);
		g->loadedDir = malloc(bytesUsed);
	}

	// If doing non Root Directory, then copy parent except name
	else
	{
		strcpy(dir[1].name, "..");
		extentCopy(dir[1].location, parent->location);
		dir[1].size = parent->size;
		dir[1].isDirectory = parent->isDirectory;
		dir[1].timeCreate = parent->timeCreate;
		dir[1].timeMod = parent->timeMod;
		dir[1].timeAccess = parent->timeAccess;
	}

	// Write directory to disk
	LBAwrite(dir, location.blockCount, location.blockNumber);

	return dir;
}

int dirLoad(directoryEntry *dir)
{

	// Load dir into memory
	free(g->loadedDir);
	g->loadedDir = malloc(dir[0].size);
	LBAread(g->loadedDir, dir[0].location.blockCount, dir[0].location.blockNumber);

	return 0;
}

int dirLocate(const char *name)
{
	// Return index where directory entry with matching name is found, -1 if it is not found
	for (int i = 0; i < g->actualDECount; i++)
	{
		printf("dirLocate run: %d, directory at %d is %s\n", i, i, g->loadedDir[i].name);
		if (strcmp(g->loadedDir[i].name, name) == 0)
		{
			printf("Item: %s found at index %d\n", name, i);
			return i;
		}
	}
	printf("That file or directory does not exist\n");
	return -1;
}

int dirCopy(directoryEntry *dest, directoryEntry *source, int index)
{

	// Copy values of source dir into dest dir[index]
	strcpy(dest[index].name, source[0].name);
	dest[index].location = source[0].location;
	dest[index].size = source[0].size;
	dest[index].isDirectory = source[0].isDirectory;
	dest[index].timeCreate = source[0].timeCreate;
	dest[index].timeMod = source[0].timeMod;
	dest[index].timeAccess = source[0].timeAccess;

	LBAwrite(dest, dest[0].location.blockCount, dest[0].location.blockNumber);

	return 0;
}

int dirDelete(directoryEntry *dir, int index)
{

	// Mark DE in dir as unused
	dir[index].name[0] = '\0';
	LBAwrite(dir, dir[0].location.blockCount, dir[0].location.blockNumber);

	return 0;
}

// Experimental function to create new directories separated from the one above to see if it works as intended
// Currently results in a segmentation fault during LBAwrite(dir, location.blockCount, location.blockNumber)
directoryEntry *newDirInit(directoryEntry *parent)
{
	printf("Start of newDirInit function\n");
	// Optimize bytes usage in block to avoid wasted bytes
	int bytesNeeded = sizeof(directoryEntry);
	int blocksNeeded = getBlocksPerByte(bytesNeeded, vcb->blockSize);
	int bytesUsed = blocksNeeded * vcb->blockSize;
	g->actualDECount = g->actualDECount + 1; // Add 1 to actual directory entry count
	// int g->actualDECount = g->actualDECount; // Testing to see if this can be used as an index
	printf("Actual Directory Entry count: %d\n", g->actualDECount);

	// Free later by caller
	printf("The malloc in here\n");
	directoryEntry *dir = malloc(bytesUsed);

	printf("The freespaceAlloc in here\n");
	// Alloc free space
	extent location = freeSpaceAlloc(blocksNeeded);

	// If doing non Root Directory, then copy parent except name

	printf("The copy statements in here\n");
	strcpy(dir[g->actualDECount].name, "..");

	printf("extent copy\n");

	// printf("The dir[at index] copies\n");
	// printf("dir[g->actualDECount]: = %d, %d\n", dir[g->actualDECount]);
	// printf("Parent is: %s\n", parent);
	// printf("parent size is %d\n", parent->size);
	extentCopy(dir[g->actualDECount].location, parent->location);

	printf("Struct copying");
	dir[g->actualDECount].size = parent->size;
	dir[g->actualDECount].isDirectory = parent->isDirectory;
	dir[g->actualDECount].timeCreate = parent->timeCreate;
	dir[g->actualDECount].timeMod = parent->timeMod;
	dir[g->actualDECount].timeAccess = parent->timeAccess;

	printf("The write statement in here\n");
	// Write directory to disk
	LBAwrite(dir, location.blockCount, location.blockNumber);

	return dir;
}