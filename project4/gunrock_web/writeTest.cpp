#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << argv[0] << ": diskImageFile Inum" << endl;
    return 1;
  }

  string diskImageFile = argv[1];
  int inum = atoi(argv[2]);

  //create Disk object
  Disk disk(diskImageFile, UFS_BLOCK_SIZE);

  // create LocalFileSystem object
  LocalFileSystem lfs(&disk);

  //create super_t instance
  super_t superBlock;
  lfs.readSuperBlock(&superBlock);

  const char* fileContents = "Lorem ipsum dolor sit amet";
  //const char* fileContents = "File contents";
  int size = strlen(fileContents);

  //write
  lfs.write(inum, fileContents, size);
  return 0;
}
