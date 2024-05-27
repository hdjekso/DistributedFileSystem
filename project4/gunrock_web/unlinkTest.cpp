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

  //const char* fileContents = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin ac lorem porttitor, auctor lorem vel, posuere eros. Integer justo metus, laoreet nec pulvinar nec, faucibus sit amet diam. Praesent vitae metus metus. Praesent ut ex at enim ullamcorper pulvinar. In neque felis, iaculis id erat quis, efficitur dapibus massa. Etiam tincidunt ultricies lorem. Morbi a magna tempus, congue sapien vel, cursus justo. Quisque scelerisque at augue a volutpat. Duis vel mi pretium, accumsan nibh in, commodo justo. Nullam lacinia, leo sit amet finibus porta, neque nibh pharetra ante, at sodales leo elit quis risus. Integer quis interdum lacus. Suspendisse consectetur congue mi quis viverra.";
  //const char* fileContents = "File contents";
  //int size = strlen(fileContents);

  //remove
  lfs.unlink(inum, filename);
  return 0;
}
