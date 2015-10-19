#ifndef DIRECTORY_H_
#define DIRECTORY_H_
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>

#define ENTRIES 16

struct Directory {
  char file[20];
  struct sockaddr_in peer;
  char used;
  time_t timestamp;
};

void DirectoryInit();
void DirectoryAdd(char *file, struct sockaddr_in peer);
struct Directory * DirectoryGet();
int DirectoryGetList(struct sockaddr_in *);
int DirectoryGetListBySearch(char *, struct sockaddr_in *);

#endif
