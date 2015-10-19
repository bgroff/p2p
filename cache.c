#include <time.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "cache.h"
#include "p2p.h"

static struct Cache cache[CACHE_ENTRIES];
static pthread_mutex_t cacheMutex;

/* Setup the directory to an empty state.
 * Make sure that the entries are marked as unused.
 * Also sets up the mutex.
 */
void CacheInit()
{
  int i;
  pthread_mutex_init(&cacheMutex, NULL);
  for (i = 0; i < CACHE_ENTRIES; i++) {
    cache[i].used = 0;
  }
}

/* Add a filename along with data to the Cache
 * Will find either an empty place in the cache or
 * find the oldest element to replace, 
 * Or return NOT_FOUND
 */
void CacheAdd(char *file, char *buf, short count)
{
  int i, old;
  time_t current;
  pthread_mutex_lock(&cacheMutex);
  time(&current);  
  // Find where to place the info
  for (i = 0; i < CACHE_ENTRIES; i++) {
    if (cache[i].used == 0) {
      old = i;
      break;
    }
    else if (cache[i].used == 1) {
      if (strcmp(file, cache[i].file) == 0) {
	time(&cache[old].timestamp);	
	pthread_mutex_unlock(&cacheMutex);
	return;
      }
    }
    else {
      if (strcmp(file, cache[i].file) == 0 && 
	  cache[i].timestamp < current) {
	current = cache[i].timestamp;
	old = i;
      }
    }
  }

  // Copy information into the list.
  if (file != NULL) {
    strcpy(cache[old].file, file);
  }

  memcpy(cache[old].data, buf, count);
  memcpy(&cache[old].count, &count, 2);
  time(&cache[old].timestamp);
  cache[i].used = 1;
  pthread_mutex_unlock(&cacheMutex);  
}


int CacheGetFile(char *file, char *buf, short *count)
{
  int i;
  time_t current;
  pthread_mutex_lock(&cacheMutex);
  time(&current);  
  for (i = 0; i < CACHE_ENTRIES; i++) {
    if (cache[i].used && strcmp(file, cache[i].file) == 0){
      cache[i].timestamp = current;
      memcpy(buf, cache[i].data, cache[i].count);
      memcpy(count, &cache[i].count, 2);
      pthread_mutex_unlock(&cacheMutex);
      return OK;
    }    
  }
  pthread_mutex_unlock(&cacheMutex);
  return NOT_FOUND;
}
