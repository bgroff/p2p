#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "rds.h"
#include "p2p.h"

static struct Entries rds[3];
pthread_mutex_t *rds_mutex;

/* Main RDS loop
 * If the entry has passed its life span
 * remove it and print a timeout message.
*/
void * RdsMain(void *args) 
{
  int retval, i;
  time_t current;
  struct timespec sleep, wait;
  sleep.tv_sec = 0;
  sleep.tv_nsec = 0x1dcd6500; // Half second
  while (1) {
    time(&current);
    retval = pthread_mutex_lock (rds_mutex);
    if (retval != 0) {
      pthread_mutex_unlock(rds_mutex);
      return;
    } 
    for (i = 0; i < 3; i++) {
      if (((current - rds[i].timestamp) > 5) && (rds[i].used)) {
	printf("File %s timed out\n", rds[i].filename);
	rds[i].used = 0;
      }
    }
    retval = pthread_mutex_unlock(rds_mutex);
    if (retval != 0)
      return;
    nanosleep(&sleep, &wait);
  }
}

/* Initialize the mutex
 * Returns OK if the mutex was created correctly
 * Otherwise it returns ERROR
*/
int RdsInit() 
{
  int retval, i;
  rds_mutex = malloc(sizeof(pthread_mutex_t));
  if (rds_mutex == NULL)
    return ERROR;
  retval = pthread_mutex_init (rds_mutex, NULL);
  if (retval != 0)
    return ERROR;
  
  for (i = 0; i < 3; i++) {
    rds[i].used = 0;
  }
  return OK;
}

/* Free the resources allocated to RDS
 * Return OK if everything went well
 * or ERROR if something went wrong
*/
int RdsDestroy()
{
  int retval;
  pthread_mutex_destroy(rds_mutex);
  if (retval != 0)
    return ERROR;
  return OK;
}

/* Add a file to the queue
 * Returns OK if everything went ok
 * Error if it did not.
*/
int RdsAdd(char filename[])
{
  int retval, i, oldest;
  time_t oldtime;
  retval = pthread_mutex_lock (rds_mutex);
  if (retval != 0) {
    printf("Mutex Lock\n");
    pthread_mutex_unlock(rds_mutex);
    return ERROR;
  }

  // If the rds is not used then use it 
  // and return OK
  for (i = 0; i < 3; i++) {
    if (!rds[i].used) {
      rds[i].used = 1;
      strcpy(rds[i].filename, filename);
      time(&oldtime);
      rds[i].timestamp = oldtime;
      pthread_mutex_unlock(rds_mutex);
      return OK;
    }
    else if (strcmp(filename, rds[i].filename) == 0) {
      pthread_mutex_unlock(rds_mutex);
      return OK;
    }
    else {
      time(&oldtime);
      if (oldtime > rds[i].timestamp) {
	oldest = i; oldtime = rds[i].timestamp;
      }      
    }
  }
  // Otherwise find the oldest file and replace it
  time(&oldtime);
  printf("Replacing %s, with %s\n", rds[oldest].filename, filename); 
  strcpy(rds[oldest].filename, filename);
  rds[oldest].timestamp = oldtime;
  
  retval = pthread_mutex_unlock(rds_mutex);
  if (retval != 0) {
    printf("Error on Mutex unlock\n");
    return ERROR;
  }
  return OK;
}

/* Removes a file from the queue
 * Returns OK if the file was removed succesfully
 * Returns NOT_FOUND if the file was not in the queue
 * Returns ERROR if something went wrong
*/
int RdsRemove(char filename[])
{
  int retval, i;
  retval = pthread_mutex_lock (rds_mutex);
  if (retval != 0) {
    pthread_mutex_unlock(rds_mutex);
    return ERROR;
  }
  
  // Loop through the rds and if the filename 
  // matches remove it. Then return OK.
  for (i = 0; i < 3; i++) {
    if (strcmp(rds[i].filename, filename) == 0) {
      rds[i].used = 0;
      strcpy(rds[i].filename, "");
      pthread_mutex_unlock(rds_mutex);
      return OK;
    }
  }
  
  retval = pthread_mutex_unlock(rds_mutex);
  if (retval != 0)
    return ERROR;
  // The file was not found in the RDS
  return NOT_FOUND;
}

int RdsSearch(char *filename)
{
   int retval, i;
  retval = pthread_mutex_lock (rds_mutex);
  if (retval != 0) {
    pthread_mutex_unlock(rds_mutex);
    return ERROR;
  }
  
  // Loop through the rds and if the filename 
  // matches then return OK.
  for (i = 0; i < 3; i++) {
    if (strcmp(rds[i].filename, filename) == 0) {
      pthread_mutex_unlock(rds_mutex);
      return OK;
    }
  }
  
  retval = pthread_mutex_unlock(rds_mutex);
  if (retval != 0)
    return ERROR;
  // The file was not found in the RDS
  return NOT_FOUND;
}
