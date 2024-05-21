#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <vector>
#include <utility>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

//custom sorting function for dirEnt pairs, based on dirName
bool comparePairs(const pair<int, string> &a, const pair<int, string> &b) {
  return strcmp(a.second.c_str(), b.second.c_str()) < 0;
}

//define new function: DFS
/*if leaf node (file) reached, return (check using .type value)
  else, use .direct[] within inode to go to direct_ent array(s) (use block # to read into buffer, then convert to direct_ent).
  direct_ent array(s) can contain curdir info, child, or parent info
  within the direct_ent curdir entry, get name of cur directory. 
  then, within each direct_ent entry stored in each direct_ent array, print inode number and name
  lastly, recurse on child(ren), by passing (all) childInodeNum into DFS().
*/
void DFS(int inodeNumber, LocalFileSystem &lfs, Disk &disk) {
  if (inodeNumber == 0) { // in rootDir
    cout << "Directory /" << endl;
  }

  inode_t curInode; 
  lfs.stat(inodeNumber, &curInode); //get inode referenced by inodeNumber

  //base case
  if (curInode.type != UFS_DIRECTORY) { //file reached
    return;
  }

  vector<pair<int, string>> dirEntsInfo; //stores inum/ name pairs of each dir entry
  //vector<int> dirEntInums; //stores inums of each dir entry for dfs recursion

  for (int i = 0; i < DIRECT_PTRS; ++i) { //iterate over inode.direct
    int blockNum = curInode.direct[i];
    if (blockNum == 0) { // no more directory entry arrays
      break; 
    }

    char block[UFS_BLOCK_SIZE];
    disk.readBlock(blockNum, block); //read all dir entries, store in `block` buffer

    //dir entries array
    dir_ent_t* dirEntries;
    
    //FIXME: potential issue?
    dirEntries = reinterpret_cast<dir_ent_t*>(block); //populate dirEntries array
    int numEntries = UFS_BLOCK_SIZE / sizeof(dir_ent_t); //# of dir entries

    for (int i = 0; i < numEntries; ++i) { //iterate through each entry to get info
      string dirName = dirEntries[i].name;
      int dirInum = dirEntries[i].inum;

      //push info into vector
      dirEntsInfo.push_back(make_pair(dirInum, dirName));
    }

    //sort dirEntsInfo vector using strcmp BASED ON DIRNAME, then print
    sort(dirEntsInfo.begin(), dirEntsInfo.end(), comparePairs);
    for (const auto &dirEnt: dirEntsInfo) { //print each entry in curDir
      cout << dirEnt.first << "\t" << dirEnt.second << endl;
    }
    cout << endl;

    //recurse
    int numInums = dirEntsInfo.size();
    for (int i = 0; i < numInums; ++i) { // iterate over inums of all dir entries
      //print the name of the directory you're about to recurse on before recursing?
      //possible that we can't get name of curDir, only child Dirs. ^ approach allows us to get name of dir we will be recursing on
      
      inode_t nextInode; 
      int nextInodeNum = dirEntsInfo[i].first;
      string nextDirName = dirEntsInfo[i].second;
      lfs.stat(nextInodeNum, &nextInode); //get inode referenced by inodeNumber

      //add extra check to make sure we only print/ recurse on dirs that aren't curDir or parentDir
      if (nextInode.type != UFS_DIRECTORY || nextDirName == "." || nextDirName == "..") { //next inode is a file OR is curDir/ parentDir
        continue;
      }else{ //recurse
        //TODO: print nextDir name, prepended by parentDirs/ fullpath
        //format: "Directory '`fullpath/curDirName`" (have a string containing parentDir names concatenated tgt)
        DFS(nextInodeNum, lfs, disk); //recurse on inum of current dir entry
      }
    }

  }
  //TODO: finish DFS()
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

  DFS(0, lfs, disk); //DFS starting from the root dir (indicate by inode 0)

  return 0;
}
