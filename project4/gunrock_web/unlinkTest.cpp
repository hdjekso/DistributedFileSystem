#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 4) {
    cout << argv[0] << ": diskImageFile parentInum fileName" << endl;
    return 1;
  }

  string diskImageFile = argv[1];
  int inum = atoi(argv[2]);
  string filename = argv[3];

  //create Disk object
  Disk disk(diskImageFile, UFS_BLOCK_SIZE);

  // create LocalFileSystem object
  LocalFileSystem lfs(&disk);

  //create super_t instance
  super_t superBlock;
  lfs.readSuperBlock(&superBlock);

  //remove
  lfs.unlink(inum, filename);
  return 0;
}
