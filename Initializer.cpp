/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "Initializer.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <sstream>

void initialize_fs(void) {
  struct stat st;
  if (stat("./DRIVE", &st) == -1) {
    mkdir("./DRIVE", 0700);
  }
  DIR *dp;
  struct dirent *ep;
  dp = opendir("./DRIVE");
  while ((ep = readdir(dp))) {
    char filepath[256];
    snprintf(filepath, sizeof filepath, "%s/%s", "DRIVE", ep->d_name);
    if (filepath[6] == '.') continue;
    printf("REMOVING %s\n", filepath);
    remove(filepath);
  }
  closedir(dp);
  char *block = new char[1024 * 1000];
  memset(block, 0, 1024 * 1000);
  for (unsigned i = 0; i < 32; i++) {
    std::ofstream file;
    std::stringstream ss;
    ss << "DRIVE/SEGMENT" << i;
    file.open(ss.str(), std::ios::binary);
    file.write(block, 1024 * 1000);
    file.close();
  }
  std::ofstream file;
  file.open("DRIVE/CHECKPOINT_REGION", std::ios::binary);
  file.write(block, 320);
  file.close();
  file.open("DRIVE/DIR", std::ios::out);
  file.close();
}
