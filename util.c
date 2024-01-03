/**************************************************************
* Project: Basic File System
* File: util.c
*
* Description: Basic File System - Utility/Helper functions
*
**************************************************************/

#include <string.h>

#include "util.h"

int getBlocksPerByte(int bytes, int blockSize) {
	return (bytes + blockSize - 1) / blockSize;
}

char *strcatReverse(char *s, char *t) {
  char *result = strcat(t, s);
  return result;
}