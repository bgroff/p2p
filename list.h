#ifndef LIST_H_
#define LIST_H_
#include "sender.h"

struct List {
  char list[56][20];
  int count;
};

void ListInit();
void ListAdd(char *);
int ListCount();
int ListSearch(char *);
struct List ListGet();
struct ListingMessage ListGetMessage();

#endif
