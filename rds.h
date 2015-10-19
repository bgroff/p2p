#ifndef RDS_H_
#define RDS_H_
#include <pthread.h>
#include <time.h>

struct Entries {
  char filename[20];
  time_t timestamp;
  char used;
};

void *RdsMain(void *args);
int RdsAdd(char filename[]);
int RdsRemove(char filename[]);
int RdsSearch(char *);
int RdsInit();
int RdsDestroy();


#endif
