/**************************************************************
* Project: Basic File System
* File: freespace.h
*
* Description: Interface of Free space functions
*
**************************************************************/

#ifndef _FREESPACE_H
#define _FREESPACE_H

#include "extent.h"

int freeSpaceInit();
extent freeSpaceAlloc(int blocksNeeded);

#endif