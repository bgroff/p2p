#include <time.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "directory.h"

static struct Directory directory[ENTRIES];
static pthread_mutex_t directoryMutex;

/* Setup the directory to an empty state.
 * Make sure that the entries are marked as unused.
 * Also sets up the mutex.
 */
void DirectoryInit()
{
  int i;
  pthread_mutex_init(&directoryMutex, NULL);
  for (i = 0; i < ENTRIES; i++) {
    directory[i].used = 0;
  }
}

/* Add a file/peer pair to the directory.
 * Will find either an empty place in the directory
 * find the oldest element to replace, or return if two
 * elemnts have the same name and peer.
 */
void DirectoryAdd(char *file, struct sockaddr_in address)
{
  int i, old;
  time_t current;
  char dest[INET_ADDRSTRLEN];
  pthread_mutex_lock(&directoryMutex);
  time(&current);  

  // Find where to place the info
  for (i = 0; i < ENTRIES; i++) {
    if (directory[i].used == 0) {
      old = i;
      break;
    }
    else if (file != NULL && directory[i].file != NULL) {
      if (strcmp(file, directory[i].file) == 0) {
	if ((directory[i].peer.sin_port == address.sin_port) 
	    && directory[i].peer.sin_addr.s_addr == address.sin_addr.s_addr) {
	  directory[i].timestamp = current;
	  pthread_mutex_unlock(&directoryMutex);
	  return;
	}
      }
    }
    else {
      if (directory[i].timestamp < current) {
	current = directory[i].timestamp;
	old = i;
      }
    }
  }

  // Copy information into the list.
  if (file != NULL) {
    strcpy(directory[old].file, file);
  }
  memcpy(&directory[old].peer, &address, sizeof(struct sockaddr_in));
  time(&directory[old].timestamp);
  directory[i].used = 1;
  pthread_mutex_unlock(&directoryMutex);  
}

/* Return a copy of the directory.
 * This way we do not need to worry about 
 * thread safety.
 */
struct Directory * DirectoryGet()
{
  return directory;
}

int DirectoryGetList(struct sockaddr_in *addresses)
{
  int i, j, count = 0;
  char unique;
  struct sockaddr_in current;
  pthread_mutex_lock(&directoryMutex);
  for (i = 0; i < ENTRIES; i++) {
    unique = 1;
    if (directory[i].used == 1) {
      current = directory[i].peer;
    }
    else
      break;
    memcpy(&addresses[count], &current, 
	   sizeof(struct sockaddr_in));
    count++;
  }
  
  pthread_mutex_unlock(&directoryMutex);
  return count;
}

int DirectoryGetListBySearch(char *search, struct sockaddr_in *addresses)
{
  int i, j, count = 0;
  struct sockaddr_in current;

  pthread_mutex_lock(&directoryMutex);
  for (i = 0; i < ENTRIES; i++) {
    if (directory[i].used == 1 
	&& strcmp(search, directory[i].file) == 0) {
      current = directory[i].peer;
    }
    else
      continue;

    memcpy(&addresses[count], &current, 
	   sizeof(struct sockaddr_in));
    count++;
  }
  
  pthread_mutex_unlock(&directoryMutex);
  return count;
}
