#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <algorithm>

#include "DistributedFileSystemService.h"
#include "ClientError.h"
#include "ufs.h"
#include "WwwFormEncodedDict.h"

using namespace std;

vector<string> splitPath(const string &path); // path parsing

DistributedFileSystemService::DistributedFileSystemService(string diskFile) : HttpService("/ds3/") {
  this->fileSystem = new LocalFileSystem(new Disk(diskFile, UFS_BLOCK_SIZE));
}  

void DistributedFileSystemService::get(HTTPRequest *request, HTTPResponse *response) { //use read()
  //response->setBody("");
  string path = request->getPath();
  if (path.size() <= 5) { // '/ds3/' not included in path
  throw ClientError::badRequest();
  } 

  path = path.substr(5); //remove '/ds3/'

  int parentInum = UFS_ROOT_DIRECTORY_INODE_NUMBER;
  vector<string> tokens = splitPath(path); //store each subpath/ token in a vector
  inode_t readInode;

  //iterate from root until desired file/ directory is reached
  for (size_t i = 0; i < tokens.size(); i++) {
    string subPathName = tokens[i];
    int curInodeNum = fileSystem->lookup(parentInum, subPathName); //get inum of current subpath/ file
    if (curInodeNum == -EINVALIDINODE || curInodeNum == -ENOTFOUND) {
      throw ClientError::notFound();
    }
    int result = fileSystem->stat(curInodeNum, &readInode); //get inode info of inode
    if (result == -EINVALIDINODE) {
      throw ClientError::notFound();
    }
    parentInum = curInodeNum; // will store readInode's num at the end
  }

  //return different stuff based on filetype
  if (readInode.type == UFS_REGULAR_FILE) {
    char buffer[readInode.size]; //store file contents
    int bytesRead = fileSystem->read(parentInum, buffer, readInode.size);
    if (bytesRead < 0) {
      throw ClientError::notFound();
    }

    string fileContents = string(buffer, bytesRead);
    response->setBody(fileContents); // return file contents
  } else if (readInode.type == UFS_DIRECTORY) {
    //print all directory entries
    //modified version of ds3ls
    vector<string> dirEntriesVector;

    char buffer[readInode.size];
    int bytesRead = fileSystem->read(parentInum, buffer, readInode.size); //read in all dirEntries
    if (bytesRead < 0) {
      throw ClientError::notFound();
    }
    dir_ent_t* dirEntries = reinterpret_cast<dir_ent_t*>(buffer); // populate dir entries array

    int numDirEntries = readInode.size / sizeof(dir_ent_t); 
    for (int i = 0; i < numDirEntries; ++i) { //iterate over all allocated blocks inode.direct,store all directory entries in dirEntsInfo
      if (dirEntries[i].inum != -1) { // is allocated
        string curDirEntryName = string(dirEntries[i].name);
        if (curDirEntryName != "." && curDirEntryName != "..") { // do not store self/ parent dir
          int curDirEntryInum = fileSystem->lookup(parentInode, curDirEntryName);
          inode_t curDirEntryInode;
          fileSystem->stat(curDirEntryInum, &curDirEntryInode); // get info on current directory entry
          //store cur dirEntry in vector
          if (curDirEntryInode.type == UFS_DIRECTORY) {
            dirEntriesVector.push_back(curDirEntryName + "/"); //add slash to name if cur dirEnt is a dir
          } else {
            dirEntriesVector.push_back(curDirEntryName);
          }
        }
      }
    }
    //sort dirEdirEntriesVector, then store in body
    stringstream body; // object to be passed to response
    sort(dirEntriesVector.begin(), dirEntriesVector.end());
    for (string& dirEnt: dirEntriesVector) { //print each entry in curDir
      body << dirEnt << "\n"; //store dir entry in body, and a newline char
    }
    response->setBody(body.str()); //set response body
  } else {
    throw ClientError::badRequest();
  }


}

void DistributedFileSystemService::put(HTTPRequest *request, HTTPResponse *response) {
  string path = request->getPath();
  if (path.size() <= 5) {
    throw ClientError::badRequest();
  }
  path = path.substr(5); // Remove "/ds3/"
  string content = request->getBody(); //get file contents from argument

  int parentInode = UFS_ROOT_DIRECTORY_INODE_NUMBER; //parent inode of the file we are trying to PUT
  vector<string> tokens = splitPath(path);
  inode_t curInode;

  fileSystem->disk->beginTransaction();
  try {
    for (size_t i = 0; i < tokens.size() - 1; i++) { //stop right before the PUT file
      string curDirName = tokens[i];
      int curDirEntryInum = fileSystem->lookup(parentInode, curDirName);
      if (curDirEntryInum == -ENOTFOUND || curDirEntryInum == -EINVALIDINODE) {
        // subDir doesn't exist, create it
        curDirEntryInum = fileSystem->create(parentInode, UFS_DIRECTORY, curDirName); //create subDir and get its inum
        if (curDirEntryInum < 0) { //error creating subDir
          throw ClientError::conflict();
        }
      } else {
        fileSystem->stat(curDirEntryInum, &curInode); //get 
        if (curInode.type != UFS_DIRECTORY) { //subdir in argument currently exists as a file
          throw ClientError::conflict();
        }
      }
      parentInode = curDirEntryInum;
    }

    //create new file/dir (at end of path) if needed
    inode_t putInode;
    int vectorLastIdx = tokens.size() - 1;
    string putFileName = tokens[vectorLastIdx];
    int putFileInum = fileSystem->lookup(parentInode, putFileName); //get inum of new file
    if (putFileInum < 0) { //new file doesn't exist, create it
      putFileInum = fileSystem->create(parentInode, UFS_REGULAR_FILE, vectorLastIdx);
      if (putFileInum < 0) { //create failed
        throw ClientError::conflict();
      }
    } else { //file/dir already exists, check if type matches
      fileSystem->stat(putFileInum, &putInode);
      if (putInode.type != UFS_REGULAR_FILE) { //is currently a dir, return error
        throw ClientError::conflict();
      }
    }

    //all error checks passed, file/dir is created, write to disk
    int result = fileSystem->write(putFileInum, content.c_str(), content.size());
    if (result == -ENOTENOUGHSPACE) {
      throw ClientError::insufficientStorage();
    }

    fileSystem->disk->commit();
  } catch (...) { //catch all errors
    fileSystem->disk->rollback();
    throw;
  }
  response->setBody("OK"); //PUT request completed
}

void DistributedFileSystemService::del(HTTPRequest *request, HTTPResponse *response) {
  response->setBody("");
}

vector<string> splitPath(const string& path) {
  vector<string> result;
  stringstream ss(path);
  string token;
  while (getline(ss, token, '/')) {
    if (!token.empty()) {
      result.push_back(token);
    }
  }
  return result;
}
