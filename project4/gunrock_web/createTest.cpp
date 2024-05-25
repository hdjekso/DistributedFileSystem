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

  //create directory
  string dirName = "testDir"
  lfs.create(0, UFS_DIRECTORY, dirName);

  //create file
  string fileName = "testFile.txt"
  lfs.create(0, UFS_DIRECTORY, fileName);

  return 0;
}
