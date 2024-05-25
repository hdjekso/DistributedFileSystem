#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <assert.h>
#include <cmath>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;

LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}

void LocalFileSystem::readSuperBlock(super_t *super) {
  //read from super block
  char block[UFS_BLOCK_SIZE];
  disk->readBlock(0, block);

  //copy data from `block` into `super`
  memcpy(super, block, sizeof(super_t));
}

//assume lookup() only looks in immediate dirEntries in dir indicated by parentInodeNumber
int LocalFileSystem::lookup(int parentInodeNumber, string name) {
  //get parent inode
  inode_t parentInode;
  int result = this->stat(parentInodeNumber, &parentInode); //stat() return -EINVALIDINODE if parentInode is invalid

  //check if stat() returned inode invalid error code
  if (result == -EINVALIDINODE) {
    return -EINVALIDINODE;
  }

  if (parentInode.type != UFS_DIRECTORY) { //file parentInode is a file
    return -ENOTFOUND; //assume file indicated by parentInode does not match `string name`
  }

  //read first block of dirEntries, and count total # of valid (allocated) dirEntries reference by parentInodeNumber
  //originally i < DIRECT_PTRS;
  for (int i = 0; i < 1; ++i) { //iterate over inode.direct
    int blockNum = parentInode.direct[i];
    if (blockNum == 0) { // no more directory entry arrays
      break; 
    }

    char block[UFS_BLOCK_SIZE];
    disk->readBlock(blockNum, block); //read all dir entries, store in `block` buffer

    //dir entries array
    dir_ent_t* dirEntries;
    
    dirEntries = reinterpret_cast<dir_ent_t*>(block); //populate dirEntries array
    int maxPossibleEntries = UFS_BLOCK_SIZE / sizeof(dir_ent_t); //max # of dir entries
    int numEntries = 0;

    //find # of valid entries
    for (int j = 0; j < maxPossibleEntries; ++j) {
      if (dirEntries[j].inum != -1) {
        numEntries++;
      }else{
        break;
      }
    }

    for (int k = 0; k < numEntries; ++k) { //iterate through each entry to get info
      string dirName = dirEntries[k].name;
      int dirInum = dirEntries[k].inum;

      if (dirName == name) { //file found, return its inode num
        return dirInum;
      }
    }
  }

  return -ENOTFOUND;
}

int LocalFileSystem::stat(int inodeNumber, inode_t *inode) {
  super_t superBlock;
  this->readSuperBlock(&superBlock);

  // Check if inodeNumber is valid
  if (inodeNumber < 0 || inodeNumber >= superBlock.num_inodes) {
    return -EINVALIDINODE; // invalid inode number
  }

  inode_t* inodes = new inode_t[superBlock.num_inodes]; //create inode table
  this->readInodeRegion(&superBlock, inodes); // populate inode table

  //copy contents into `inode` argument
  *inode = inodes[inodeNumber];

  delete[] inodes;
  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
  bool invalidSize = false;
  
  super_t superBlock;
  this->readSuperBlock(&superBlock);

  /*inode_t* inodes = new inode_t[superBlock.num_inodes]; //create inode table
  this->readInodeRegion(&superBlock, inodes); // populate inode table

  inode_t inode = inodes[inodeNumber]; // get inode from inode table using inodeNumber*/
  inode_t inode;
  this->stat(inodeNumber, &inode); //get inode referenced by inodeNumber

  int fileBlockNum = 0; // used to access physical block #
  int blockNum = inode.direct[fileBlockNum]; //get physical block # using file block # (0)

  // Check if inodeNumber is valid
  if (inodeNumber < 0 || inodeNumber >= superBlock.num_inodes) {
    //delete[] inodes;
    return -EINVALIDINODE; // Return error code for invalid inode number
  }
  //check if size is valid
  if (size < 0) {
    return -EINVALIDSIZE;
  }

  // prevent reading after eof
  int fileSize = inode.size;
  if (size > fileSize) { 
    invalidSize = true;
    size = fileSize;
  }
  /////////////////////////////////////
  int bytesRemaining = size; //number of bytes still needed to fill up buffer
  int bytesRead = 0; // used to indicate where to cpy into buffer
  while (bytesRemaining > 0) {
    char block[UFS_BLOCK_SIZE];
    blockNum = inode.direct[fileBlockNum]; //access next physical block #
    disk->readBlock(blockNum, block);
    
    // Calculate how many bytes to copy from the block that was just read
    int bytesToCopy = min(bytesRemaining, UFS_BLOCK_SIZE);
    memcpy(static_cast<char *>(buffer) + bytesRead, block, bytesToCopy); // copy `bytesToCopy` bytes from block into buffer + offset
    
    //update counters
    bytesRemaining -= bytesToCopy;
    bytesRead += bytesToCopy; 
    fileBlockNum++; 
  }

  //delete[] inodes;
  if (bytesRead != size) { //error checking
    cerr << "error with read(): incorrect number of bytes read; bytesRead: " << bytesRead << ", size: " << size << endl;
    return 1;
  }
  if (invalidSize) {
    return -EINVALIDSIZE;
  }
  return size;
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  /*ERROR CHECKS*/
  //check if parentInode exists
  inode_t parentInode;
  int result = this->stat(parentInodeNumber, &parentInode); //stat() returns -EINVALIDINODE if parentInode is invalid

  //check if stat() returned -EINVALIDINODE
  if (result == -EINVALIDINODE) {
    return -EINVALIDINODE;
  }

  //if length name exceeds DIR_ENT_NAME_SIZE, return -EINVALIDNAME
  if (name.length() >= DIR_ENT_NAME_SIZE) { //char[] is terminated by /0, 27 chars only
    return -EINVALIDNAME;
  }

  //check if file/dir we are trying to create exists using inum = lookup(name)
  //if it exists, check if type matches using stat(inum) and .type. return success (inum) or -EINVALIDTYPE depending
  int newFileInum = this->lookup(parentInodeNumber, name);
  if (newFileInum != -ENOTFOUND) { //file exists
    //get inode info of newFile
    inode_t newFileInode;
    this->stat(newFileInum, &newFileInode);

    //check if type matches argument
    if (newFileInode.type == type) {
      return newFileInum; //success
    }else{
      return -EINVALIDTYPE;
    }
  }
  
  //check if space exists in disk using diskHasSpace()
  super_t superBlock;
  this->readSuperBlock(&superBlock);
  //assume new file only needs 1 block of space
  if (!this->diskHasSpace(&superBlock, 1, UFS_BLOCK_SIZE, 0)) {
    return -ENOTENOUGHSPACE;
  }
  /* ERROR CHECKS OVER*/
  
  //create file
  //creating file/dir: assign newFile inode -> assign newFile data -> update parent data -> update parent Inode -> update inode bitmap -> update data bitmap

  //PART 1: find free inode #, update inode bitmap
  ////////////////////////////////////////////////////////////
  //assign inode number for newFile

  //create buffer to store inode bitmap
  unsigned char inodeBitmapBuffer[superBlock.inode_bitmap_len * UFS_BLOCK_SIZE]; //buffer to store bitmap
  int inodeBitmapSize = superBlock.inode_bitmap_len * UFS_BLOCK_SIZE;
  this->readInodeBitmap(&superBlock, inodeBitmapBuffer);

  //find free inode num in bitmap. each bit represents one inode
  bool freeInodeFound = false;
  int freeInodeNum = -1;
  for (int byteIdx = 0; byteIdx < inodeBitmapSize; ++byteIdx) {
    for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
      if ((inodeBitmapBuffer[byteIdx] & (1 << bitIdx)) == 0) { //apply mask with 1 at position `bitIndex`, and AND it with current byte
        freeInodeNum = byteIdx * 8 + bitIdx; //free inode num found
        inodeBitmapBuffer[byteIdx] |= (1 << bitIdx); // Mark the inode as used (set bit to 1)
        freeInodeFound = true;
        break;
      }
    }
    if (freeInodeFound) {
      break;
    }
  }
  this->writeInodeBitmap(&superBlock, inodeBitmapBuffer); //update inode bitmap in disk
  //bonus error check
  if (freeInodeNum == -1) { // No free inode found
      return -ENOTENOUGHSPACE;
  }

  //PART 2: find free block # within inode, update data bitmap. only do this for dir
  //////////////////////////////////////////////////////////////////
  //create buffer to store data bitmap
  int freeBlockNum = -1;

  unsigned char dataBitmapBuffer[superBlock.data_bitmap_len * UFS_BLOCK_SIZE]; //buffer to store bitmap
  int dataBitmapSize = superBlock.data_bitmap_len * UFS_BLOCK_SIZE;
  this->readDataBitmap(&superBlock, dataBitmapBuffer);
  if (type == UFS_DIRECTORY) {
    //find free block num in bitmap. each bit represents one data block
    bool freeBlockNumFound = false;
    for (int byteIdx = 0; byteIdx < dataBitmapSize; ++byteIdx) {
      for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
        if ((dataBitmapBuffer[byteIdx] & (1 << bitIdx)) == 0) { //apply mask with 1 at position `bitIndex`, and AND it with current byte
          //FIXME: assume block numbers are indexed starting from 4
          freeBlockNum = (byteIdx * 8 + bitIdx) + superBlock.data_region_addr; //free block num found
          dataBitmapBuffer[byteIdx] |= (1 << bitIdx); // Mark the data block as used (set bit to 1)
          freeBlockNumFound = true;
          break;
        }
      }
      if (freeBlockNumFound) {
        break;
      }
    }
    this->writeDataBitmap(&superBlock, dataBitmapBuffer); //update data bitmap in disk
    //bonus error check
    if (freeBlockNum == -1) { // No free block num found
        return -ENOTENOUGHSPACE;
    }
  }

  //PART 3: assign newFile parameters  (type, size, .direct[]) to the free inode
  /////////////////////////////////////////////////////////////////////////
  //read in entire inode table
  inode_t* inodes = new inode_t[superBlock.num_inodes]; //create inode table
  this->readInodeRegion(&superBlock, inodes); // populate inode table

  inode_t& createFileInode = inodes[freeInodeNum]; //get the inode we want to update (reference: updating `createFileInode` updates `inodes` as well)
  inode_t& newFileParentInode = inodes[parentInodeNumber]; // create another instance of parent inode, this time as a reference to `inodes`

  //assign inode_t type
  createFileInode.type = type;
  //assign inode_t .direct[]
  //FIXME: potential issue with memset
  //memset(createFileInode.direct, 0, sizeof(createFileInode.direct)); //initialize all elements inside .direct[] to 0
  if (type == UFS_DIRECTORY) {
    createFileInode.direct[0] = unsigned(freeBlockNum);
  }

  //assign inode_t size
  if (type == UFS_REGULAR_FILE) {
    createFileInode.size = 0;
  }else{ //directory
    //TODO: ensure .size value is accurate
    createFileInode.size = 2 * sizeof(dir_ent_t); //new dir will have two entries: "." and ".."
  }
  this->writeInodeRegion(&superBlock, inodes); //write inode table changes to disk

  //PART 4: write contents of newFile to data region
  //////////////////////////////////////////////////////////////////////////////////
  if (type == UFS_DIRECTORY) {
    char block[UFS_BLOCK_SIZE];
    disk->readBlock(freeBlockNum, block); //read contents of free data block, store in `block` buffer

    //dir entries array. 
    dir_ent_t* dirEntries;
    dirEntries = reinterpret_cast<dir_ent_t*>(block); //populate dirEntries array. modifying dirEntries modifies `block` as well
    int maxPossibleEntries = UFS_BLOCK_SIZE / sizeof(dir_ent_t); //max # of dir entries
    //assign initial directory entries if type = dir (dir_ent_t) (i.e. update createNewInode.direct[])

    //update free data block (a data block within data region): find first 2 unallocated dir entries within free data block (assume it to be [0] and [1]) and update them. set all other dir entries to 'unalloacted'
    strcpy(dirEntries[0].name, "."); 
    dirEntries[0].inum = freeInodeNum;
    strcpy(dirEntries[1].name, "..");
    dirEntries[1].inum = parentInodeNumber;
    for (int i = 2; i < maxPossibleEntries; ++i) { //set remaining dir entries to 'unallocated'
      dirEntries[i].inum = -1;
    }
    disk->writeBlock(freeBlockNum, block); //write changes to block to data region
  }else { //filetype = file
    //DO NOTHING
  }

  //PART 5: update parent data: update parent Inode to contain new File as a dir entry within a block in .direct[]
  //////////////////////////////////////////////////////////////////////////////////////////////
  //find first unallocated dir entry in parentInode, add new file/dir's blockNum to it
  bool parentInodeUpdated = false;
  int activeBlocks = parentInode.size / UFS_BLOCK_SIZE; //allocated blocks
  if (parentInode.size % UFS_BLOCK_SIZE != 0) {
    activeBlocks++;
  }
  for (int i = 0; i < activeBlocks; ++i) { //iterate over inode.direct
    int curBlockNum = newFileParentInode.direct[i];

    char parentBlock[UFS_BLOCK_SIZE];
    disk->readBlock(curBlockNum, parentBlock); //read all dir entries, store in `block` buffer

    dir_ent_t* parentDirEntries;
    parentDirEntries = reinterpret_cast<dir_ent_t*>(parentBlock);

    //find 1st unallocated dir entry in current block
    int allEntriesInBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t); //equivalent to maxPossibleEntries
    for (int j = 0; j < allEntriesInBlock; ++j) {
      if (parentDirEntries[j].inum == -1) { //unallocated dir entry found, allocate it (assign new file/ dir info to it)
        parentDirEntries[j].inum = freeInodeNum;
        strcpy(parentDirEntries[j].name, name.c_str());
        disk->writeBlock(curBlockNum, parentBlock);
        parentInodeUpdated = true;
        break;
      }
    }
    if (parentInodeUpdated) {
      break;
    }
  }
  if (!parentInodeUpdated) { //need to find a new block to store new file/dir
    //allocate new block to parent inode, store new file/ dir within
    bool parentFreeBlockNumFound = false;
    this->readDataBitmap(&superBlock, dataBitmapBuffer); //make sure dataBitmapBuffer is up to date
    int parentFreeBlockNum = -1;
    
    for (int byteIdx = 0; byteIdx < dataBitmapSize; ++byteIdx) {
      for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
        if ((dataBitmapBuffer[byteIdx] & (1 << bitIdx)) == 0) { //apply mask with 1 at position `bitIndex`, and AND it with current byte
          //FIXME: assume block numbers are indexed starting from 4
          parentFreeBlockNum = (byteIdx * 8 + bitIdx) + superBlock.data_region_addr ; //free block num found
          dataBitmapBuffer[byteIdx] |= (1 << bitIdx); // Mark the data block as used (set bit to 1)
          parentFreeBlockNumFound = true;
          break;
        }
      }
      if (parentFreeBlockNumFound) {
        break;
      }
    }
    if (parentFreeBlockNum == -1) { // No free block num found
      return -ENOTENOUGHSPACE;
    }
    this->writeDataBitmap(&superBlock, dataBitmapBuffer); //update data bitmap in disk
    newFileParentInode.direct[activeBlocks] = parentFreeBlockNum; //assign new block num to element in .direct[]
    this->writeInodeRegion(&superBlock, inodes); //write updated inode info to inode region

    //assign file/dir info to dir entry
    char newParentBlock[UFS_BLOCK_SIZE];
    disk->readBlock(parentFreeBlockNum, newParentBlock); //read all dir entries of this new (empty), store in `block` buffer

    dir_ent_t* parentDirEntries;
    parentDirEntries = reinterpret_cast<dir_ent_t*>(newParentBlock);

    int allEntriesInBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t); //equivalent to maxPossibleEntries
    for (int j = 0; j < allEntriesInBlock; ++j) {
      if (parentDirEntries[j].inum == -1) { //unallocated dir entry found, allocate it (assign new file/ dir info to it)
        parentDirEntries[j].inum = freeInodeNum;
        strcpy(parentDirEntries[j].name, name.c_str());
        disk->writeBlock(parentFreeBlockNum, newParentBlock);
        break;
      }
    }    
  }


  //PART 6: update parent inode parameter(s)
  //////////////////////////////////////////////////////////////////////////////////////////////
  //update newFileParentInode's .size parameter
  newFileParentInode.size += sizeof(dir_ent_t);
  this->writeInodeRegion(&superBlock, inodes);

  delete[] inodes;
  return 0;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  //modifying parent info needed: .size is determined by size of all its dir entries
  //will writeBlock() automatically write all contents into each dir_ent_t?

  /*ERROR CHECKS*/

  //check if inode exists
  inode_t curInode;
  int result = this->stat(inodeNumber, &curInode); //stat() returns -EINVALIDINODE if parentInode is invalid

  //check if stat() returned -EINVALIDINODE
  if (result == -EINVALIDINODE) {
    return -EINVALIDINODE;
  }

  //size check
  if (size < 0) {
    return -EINVALIDSIZE;
  }

  //type check
  if (curInode.type != UFS_REGULAR_FILE) {
    return -EINVALIDTYPE;
  }

  //storage check
  super_t superBlock;
  this->readSuperBlock(&superBlock);
  int curNumBlocksOccupied = curInode.size / UFS_BLOCK_SIZE; //current # of blocks file content is occupying
  if (curInode.size % UFS_BLOCK_SIZE != 0) {
    curNumBlocksOccupied++;
  }
  int newNumBlocksOccupied = size / UFS_BLOCK_SIZE; //current # of blocks file content is occupying
  if (size % UFS_BLOCK_SIZE != 0) {
    newNumBlocksOccupied++;
  }
  int additionalBlocksNeeded = newNumBlocksOccupied - curNumBlocksOccupied;

  if (additionalBlocksNeeded > 0 && !this->diskHasSpace(&superBlock, 0, 0, additionalBlocksNeeded)) {
    return -ENOTENOUGHSPACE;
  }

  return 0;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  //error checks
  //dir not empty -> dir to be removed has subdirectories/ files within

  //get inode number of file using lookup()
  //empty inode contents of file
  //find dir entry containing the file within a block in parentInode's .direct[] array. remove dir entry within block
  return 0;
}

//helper functions
void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
  disk->readBlock(super->inode_bitmap_addr, inodeBitmap);
}

 void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap) {
  disk->readBlock(super->data_bitmap_addr, dataBitmap);
}

//read all inodes, and store in `inodes`
void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes) {
  int inodeRegionSize = super->inode_region_len * UFS_BLOCK_SIZE; //get size of inode region in bytes
  unsigned char* inodeRegionBuffer = new unsigned char[inodeRegionSize]; //buffer to store inode region

  for (int i = 0; i < super->inode_region_len; ++i) { //read inode blocks one at a time
    disk->readBlock(super->inode_region_addr + i, inodeRegionBuffer + (i * UFS_BLOCK_SIZE));
  }
  memcpy(inodes, inodeRegionBuffer, inodeRegionSize); //copy buffer contents into `inodes` array
  delete[] inodeRegionBuffer;
}

bool LocalFileSystem::diskHasSpace(super_t *super, int numInodesNeeded, int numDataBytesNeeded, int numDataBlocksNeeded) {
  if (numDataBlocksNeeded == 0) { //assume 0 indicates numDataBlocksNeeded is not specified
    numDataBlocksNeeded = (numDataBytesNeeded / UFS_BLOCK_SIZE);
    if (numDataBytesNeeded % UFS_BLOCK_SIZE != 0) {
    numDataBlocksNeeded++;
    }
  }

  super_t superBlock;
  this->readSuperBlock(&superBlock);

  //create buffer to store inode bitmap
  unsigned char inodeBitmapBuffer[superBlock.inode_bitmap_len * UFS_BLOCK_SIZE]; //buffer to store bitmap
  int inodeBitmapSize = superBlock.inode_bitmap_len * UFS_BLOCK_SIZE; // in bytes
  this->readInodeBitmap(&superBlock, inodeBitmapBuffer);

  //count num free inodes
  int freeInodeCount = 0;
  for (int byteIdx = 0; byteIdx < inodeBitmapSize; ++byteIdx) {
    for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
      if ((inodeBitmapBuffer[byteIdx] & (1 << bitIdx)) == 0) { //apply mask with 1 at position `bitIndex`, and AND it with current byte
        freeInodeCount++; //free inode found
      }
    }
  }

  //create buffer to store data bitmap
  unsigned char dataBitmapBuffer[superBlock.data_bitmap_len * UFS_BLOCK_SIZE]; //buffer to store bitmap
  int dataBitmapSize = superBlock.data_bitmap_len * UFS_BLOCK_SIZE;
  this->readDataBitmap(&superBlock, dataBitmapBuffer);

  //count num free data blocks. each bit in bitmap represents a data block
  int freeDataBlocksCount = 0;
  for (int byteIdx = 0; byteIdx < dataBitmapSize; ++byteIdx) {
    for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
      if ((dataBitmapBuffer[byteIdx] & (1 << bitIdx)) == 0) { //apply mask with 1 at position `bitIndex`, and AND it with current byte
        freeDataBlocksCount++; //free data block found
      }
    }
  }

  if (freeDataBlocksCount >= numDataBlocksNeeded && freeInodeCount >= numInodesNeeded) {
    return true;
  }else{
    return false;
  }
}

//take `inodeBitMap` and write it to disk
void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
  //int inodeBitmapSize = super->inode_bitmap_len * UFS_BLOCK_SIZE;

  //iterate through every block of the inode bitmap in disk
  for (int i = 0; i < super->inode_bitmap_len; ++i) {
    //write each block from `inodeBitmap` to inode bitmap in disk using offset i
    disk->writeBlock(super->inode_bitmap_addr + i,  inodeBitmap + (i * UFS_BLOCK_SIZE));
  }
}

//same implementation as above
void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap) {
  //iterate through every block of the data bitmap in disk
  for (int i = 0; i < super->data_bitmap_len; ++i) {
    //write each block from `dataBitmap` to data bitmap in disk using offset i
    disk->writeBlock(super->data_bitmap_addr + i,  dataBitmap + (i * UFS_BLOCK_SIZE));
  } 
}

//use content stored in `inodes` and write to inode region in disk
void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes) {
  for (int i = 0; i < super->inode_region_len; ++i) {
    disk->writeBlock(super->inode_region_addr + i, inodes + (i * UFS_BLOCK_SIZE));
  }
}
