#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "p2p.h"
#include "rds.h"
#include "sender.h"
#include "list.h"
#include "directory.h"
#include "receiver.h"

int sock;

int main(int argc, char *argv[])
{
  int len, i;
  int port, count = 2;
  char *result = NULL;
  char address[16];
  struct sockaddr_in peer;
  pthread_t *rdsThread = malloc(sizeof(pthread_t));
  pthread_t *listingThread = malloc(sizeof(pthread_t));
  pthread_t *receiverThread = malloc(sizeof(pthread_t));

  if (argc < 2) { 
    printf("Usage: portnumber [ip/port]\n");
    exit(0);
  }
  
  ListInit();
  DirectoryInit();
  CacheInit();
  port = atoi(argv[1]);
  
  while ((count < argc) && (count < 20)) {
    result = strchr(argv[count], '/');
    if (result == NULL) {
      ListAdd(argv[count]);
    }
    else {
      memset(&peer, 0, sizeof(struct sockaddr_in));
      peer.sin_family = AF_INET;
      peer.sin_port = htons(atoi(result+1));	
      len = strlen(argv[count]) - strlen(result);
      strncpy(address, argv[count], len);
      address[len] = '\0';
      if (inet_aton(address, &peer.sin_addr) == 0)
	return -1;
      DirectoryAdd(NULL, peer);
    }
    count++;
  }
 
  // Start the application threads
  RdsInit();
  ReceiverInit(port);
  pthread_create(rdsThread, NULL, RdsMain, NULL);
  pthread_create(listingThread, NULL, SendListing, NULL);
  pthread_create(receiverThread, NULL, Receiver, (void *)&port);
  ClientMain();
  exit(1);
}

int ClientMain()
{
  char buf[20];
  while(1) {
    scanf("%s",buf);
    SendRequest(buf);
  }
}
