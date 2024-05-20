#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

//define new function: DFS
/*if leaf node (file) reached, return (check using .type value)
  else, use .direct[] within inode to go to direct_ent object(s) (use block # to read into buffer, then convert to direct_ent).
  direct_ent object(s) can contain curdir info, child, or parent info
  within the direct_ent curdir entry, get name of cur directory. 
  then, within each direct_ent entry stored in direct[], print inode number and name
  lastly, recurse on child(ren), by passing (all) childInodeNum into DFS().
*/
void DFS(int inodeNumber, LocalFileSystem &lfs) {
  inode_t curInode; 
  lfs.stat(inodeNumber, &curInode); //get inode referenced by inodeNumber

  //base case
  if (curInode.type != UFS_DIRECTORY) { //file reached
    return;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << argv[0] << ": diskImageFile" << endl;
    return 1;
  }

  string diskImageFile = argv[1];
  Disk disk(diskImageFile, UFS_BLOCK_SIZE);
  // Create LocalFileSystem object
  LocalFileSystem lfs(&disk);

  DFS(0, lfs); //DFS starting from the root dir (indicate by inode 0)

  return 0;
}
