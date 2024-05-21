#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << argv[0] << ": diskImageFile" << endl;
    return 1;
  }

  string diskImageFile = argv[1];

  //create Disk object
  Disk disk(diskImageFile, UFS_BLOCK_SIZE);

  // create LocalFileSystem object
  LocalFileSystem lfs(&disk);

  //create super_t instance
  super_t superBlock;
  lfs.readSuperBlock(&superBlock);

  //print metadata
  cout << "Super" << endl;
  cout << "inode_region_addr " << superBlock.inode_region_addr << endl;
  cout << "data_region_addr " << superBlock.data_region_addr << endl;
  cout << endl;

  //inode bitmap
  cout << "Inode bitmap" << endl;

  //create buffer to store inode bitmap
  unsigned char inodeBitmapBuffer[superBlock.inode_bitmap_len * UFS_BLOCK_SIZE]; //buffer to store bitmap
  int inodeBitmapSize = superBlock.inode_bitmap_len * UFS_BLOCK_SIZE;
  lfs.readInodeBitmap(&superBlock, inodeBitmapBuffer); // read bitmap data into buffer
  
  //print inode bitmap
  for (int i = 0; i < inodeBitmapSize; ++i) {
    cout << static_cast<unsigned int>(inodeBitmapBuffer[i]) << " ";
  }
  cout << endl;
  cout << endl;

  //data region bitmap
  cout << "Data bitmap" << endl;

  //create buffer to store data bitmap
  unsigned char dataBitmapBuffer[superBlock.data_bitmap_len * UFS_BLOCK_SIZE];
  int dataBitmapSize = superBlock.data_bitmap_len * UFS_BLOCK_SIZE;
  lfs.readDataBitmap(&superBlock, dataBitmapBuffer); // read bitmap data into buffer

  //print
  for (int i = 0; i < dataBitmapSize; ++i) {
    cout << static_cast<unsigned int>(dataBitmapBuffer[i]) << " ";
  }
  cout << endl;

  return 0;
}
