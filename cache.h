#ifndef CACHE_H_
#define CACHE_H_
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>

#define CACHE_ENTRIES 5
struct Cache {
  char file[20];
  char data[1200];
  char used;
  short count;
  time_t timestamp;
};

void CacheInit();
void CacheAdd(char *, char *, short);
int CacheGetFile(char *, char *, short *);

#endif
