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
void DFS(int inodeNumber, LocalFileSystem &lfs, Disk &disk, string fullPath) {
  if (inodeNumber == 0) { // in rootDir
    cout << "Directory /" << endl;
  }

  inode_t curInode; 
  lfs.stat(inodeNumber, &curInode); //get inode referenced by inodeNumber

  vector<pair<int, string>> dirEntsInfo;
  //base case
  if (curInode.type != UFS_DIRECTORY) { //file reached
    return;
  }

   //stores inum/ name pairs of each dir entry
  //vector<int> dirEntInums; //stores inums of each dir entry for dfs recursion

  int activeBlocks = curInode.size / UFS_BLOCK_SIZE; //allocated blocks
  if (curInode.size % UFS_BLOCK_SIZE != 0) {
    activeBlocks++;
  }
  int numDirEntries = curInode.size / sizeof(dir_ent_t); //remaining entries that need to be read

  for (int i = 0; i < activeBlocks; ++i) { //iterate over all allocated blocks inode.direct,store all directory entries in dirEntsInfo
    if (numDirEntries == 0) {
      break;
    }
    int blockNum = curInode.direct[i];
    if (blockNum == 0) { // no more directory entry arrays
      break; 
    }

    char block[UFS_BLOCK_SIZE];
    disk.readBlock(blockNum, block); //read all dir entries, store in `block` buffer

    //dir entries array
    dir_ent_t* dirEntries;
    
    dirEntries = reinterpret_cast<dir_ent_t*>(block); //populate dirEntries array
    int maxPossibleEntries = UFS_BLOCK_SIZE / sizeof(dir_ent_t); //max # of dir entries
    /*int numEntries = 0;

    //find # of valid entries
    for (int j = 0; j < maxPossibleEntries; ++j) {
      if (dirEntries[j].inum != -1) {
        numEntries++;
      }else{
        break;
      }
    }*/

    for (int k = 0; k < min(maxPossibleEntries, numDirEntries); ++k) { //iterate through each entry to get info
      string dirName = dirEntries[k].name;
      int dirInum = dirEntries[k].inum;

      //push info into vector
      dirEntsInfo.push_back(make_pair(dirInum, dirName));
    }
    numDirEntries -= min(maxPossibleEntries, numDirEntries);
  }
  //sort dirEntsInfo vector using strcmp BASED ON DIRNAME, then print
  sort(dirEntsInfo.begin(), dirEntsInfo.end(), comparePairs);
  for (const auto &dirEnt: dirEntsInfo) { //print each entry in curDir
    cout << dirEnt.first << "\t" << dirEnt.second << endl;
  }

  //recurse
  int numInums = dirEntsInfo.size();
  for (int l = 0; l < numInums; ++l) { // iterate over inums of all dir entries
    //print the name of the directory you're about to recurse on before recursing?
    //possible that we can't get name of curDir, only child Dirs. ^ approach allows us to get name of dir we will be recursing on
    
    inode_t nextInode; 
    int nextInodeNum = dirEntsInfo[l].first;
    string nextDirName = dirEntsInfo[l].second;
    lfs.stat(nextInodeNum, &nextInode); //get inode referenced by inodeNumber

    //add extra check to make sure we only print/ recurse on dirs that aren't curDir or parentDir
    if (nextInode.type != UFS_DIRECTORY || nextDirName == "." || nextDirName == "..") { //next inode is a file OR is curDir/ parentDir
      continue;
    }else { //recurse
      //TODO: print nextDir name, prepended by parentDirs/ fullpath
      //format: "Directory '`fullpath/curDirName`" (have a string containing parentDir names concatenated tgt)
      cout << endl;
      string newFullPath = fullPath + nextDirName + "/";
      cout << "Directory " << newFullPath << endl;
      DFS(nextInodeNum, lfs, disk, newFullPath); //recurse on inum of current dir entry
    }
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

  DFS(0, lfs, disk, "/"); //DFS starting from the root dir (indicate by inode 0)
  cout << endl;

  return 0;
}
