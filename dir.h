/**************************************************************
*
* Description: Directory Entry structure
*
**************************************************************/
#ifndef _DIR_H
#define _DIR_H

#include <time.h>

#include "extent.h"

#define FILENAMESIZE 256
#define STARTDE 50

typedef struct directoryEntry {
	char name[FILENAMESIZE];      // Name of the file, 255 char max
  extent location;              // Keep track of location on disk
  int size;                     // Size of the file in bytes
  int isDirectory;              // 0 for non-dir, and 1 for dirDescription: Interface of Directory and DE functions
  time_t timeCreate;            // Time when file was created
  time_t timeMod;               // Time when file was modified
  time_t timeAccess;            // Time when file was accessed
} directoryEntry;

directoryEntry *dirInit(int baseNumDE, directoryEntry *parent);

// Load dir into memory
int dirLoad(directoryEntry *dir);

// Lookup entry index in directory
int dirLocate(const char *name);

// Copy dir contents
int dirCopy(directoryEntry *dest, directoryEntry *source, int index);

// Delete DE within a directory
int dirDelete(directoryEntry *dir, int index);

#endif
