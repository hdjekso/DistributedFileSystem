#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << argv[0] << ": diskImageFile inodeNumber" << endl;
    return 1;
  }

  string diskImageFile = argv[1];
  int inodeNumber = atoi(argv[2]);

  
  //create Disk object
  Disk disk(diskImageFile, UFS_BLOCK_SIZE);

  // create LocalFileSystem object
  LocalFileSystem lfs(&disk);

  //////////////////////////////////// CODE BELOW ACHIEVED BY STAT()
  /*// Read super block
  super_t superBlock;
  lfs.readSuperBlock(&superBlock);

  // Allocate memory for inodes array
  inode_t *inodes = new inode_t[superBlock.num_inodes];

  // Populate inode table
  lfs.readInodeRegion(&superBlock, inodes);

  // Check if inodeNumber is valid
  if (inodeNumber < 0 || inodeNumber >= superBlock.num_inodes) {
      // Free allocated memory
      delete[] inodes;
      return -EINVALIDINODE; // Return error code for invalid inode number
  }

  // Get inode from inode table using inodeNumber
  inode_t inode = inodes[inodeNumber];*/
  /////////////////////////////////////////////////////////////////////

  inode_t inode; 
  lfs.stat(inodeNumber, &inode); //get inode referenced by inodeNumber

  //get size of file referenced by inode
  int fileSize = inode.size;
  //create buffer
  char* buffer = new char[fileSize];

  //call read(), read in entire file
  lfs.read(inodeNumber, buffer, fileSize);

  //print
  cout << "File blocks" << endl;

  //print disk block #'s
  int numBlocks = fileSize / UFS_BLOCK_SIZE; //the # of blocks the file is stored in
  //int* diskBlockNums = new int[numBlocks]; //array storing disk block #s
  for (int i = 0; i <= numBlocks; ++i) {
    //diskBlockNums[i] = inode.direct[i];
    cout << inode.direct[i] << endl;
  }
  cout << endl;

  //print file contents
  cout << "File data" << endl;
  for (int i = 0; i < fileSize; ++i) {
    cout << buffer[i];
  }
  cout << endl;

  //delete[] inodes;
  delete[] buffer;
  //delete[] diskBlockNums;
  return 0;
}
