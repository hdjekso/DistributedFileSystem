#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 5) {
    cout << argv[0] << ": diskImageFile Inum Filetype Filename" << endl;
    return 1;
  }

  string diskImageFile = argv[1];
  int inum = atoi(argv[2]);
  string filetype_name = argv[3];
  int filetype;
  string filename = argv[4];

  if (filetype_name == "file") {
    filetype = UFS_REGULAR_FILE;
  }else {
    filetype = UFS_DIRECTORY;
  }

  //create Disk object
  Disk disk(diskImageFile, UFS_BLOCK_SIZE);

  // create LocalFileSystem object
  LocalFileSystem lfs(&disk);

  //create super_t instance
  super_t superBlock;
  lfs.readSuperBlock(&superBlock);

  //create file/dir
  lfs.create(inum, filetype, filename);

  //create file
  /*string fileName = "testFile.txt";
  lfs.create(0, UFS_REGULAR_FILE, fileName);*/

  return 0;
}
