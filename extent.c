/**************************************************************
*
* Description: Basic File System - Extent functions
*
**************************************************************/

#include "extent.h"

int extentCopy(extent dest, extent source) {

  dest.blockNumber = source.blockNumber;
  dest.blockCount = source.blockCount;

  return 0;
}