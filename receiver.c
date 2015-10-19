#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "receiver.h"
#include "list.h"
#include "cache.h"
#include "p2p.h"

extern int sock;

void ReceiverInit(int port)
{
  struct sockaddr_in host;
  if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
    fprintf(stderr, "Error Creating Socket\n");
    exit(1);
  }
  memset((char *) &host, 0, sizeof(host));
  host.sin_family = AF_INET;
  host.sin_port = htons(port);
  host.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sock, (struct sockaddr *)&host, sizeof(host))==-1) {
    fprintf(stderr, "Error Binding Socket\n");
    exit(1);
  }
}

void *Receiver(void *args)
{
  int i;
  unsigned int len = sizeof(struct sockaddr_in);
  
  unsigned char buf[BUFLEN], *packet;
  struct sockaddr_in host, dest;
  struct Message *message;
  pthread_t *thread = malloc(sizeof(pthread_t));
  
   // Receive Messages and spawn worker threads.	
   while (1) {
     if (recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *)&dest, &len)==-1)
       exit(1);
     packet = malloc(BUFLEN);
     message = malloc(sizeof(struct Message));
     memcpy(packet, buf, BUFLEN);
     memcpy(&(message->address), &dest, len);
     message->packet = packet;
     pthread_create(thread, NULL, ParseMessages, (void *)message);
   }
   
   close(sock);
   return;
}

void * ParseMessages(void *args)
{
  unsigned char cont, code;
  struct Message *message = (struct Message *)args;
  unsigned char *packet = message->packet;
  cont = (unsigned char)packet[0];
  code = (unsigned char)packet[1];
  switch(cont) {
  case CONTROL:
    switch (code) {
    case LISTING:
      ParseListing(message);
      break;
    case REQUEST:
      ParseRequest(message);
      break;
    case TRY:
      ParseTry(message);
      break;
    }
    break;
  case DATA:
    ParseData(message);
    break;
  }
  free(message->packet);
  free(message);
}

void ParseListing(struct Message *message)
{
  int i, j, k;
  short count; 
  char buf[20], *ptr;
  struct ListingMessage *list = (struct ListingMessage *)message->packet;
  count = ntohs(list->count);
  ptr = (char *) message->packet + 4; // Point to the files in the packet
  
  j = 0;
  for (i = 0; i < count; i++) { // Iterate through the files
    k = 0;
    while(ptr[j] != '\0') { // By copying each bit
      buf[k] = ptr[j];
      j++; k++;
    }
    buf[k] = '\0'; j++; // Skip over the NULL bit
    DirectoryAdd(buf, message->address);
  }
}

void ParseRequest(struct Message *message)
{
  char buf[20], datagram[1200], *ptr;
  unsigned short len, buflen, size, data = 0;
  struct sockaddr_in list[20];
  FILE *file = NULL;   
  ptr = message->packet + 2;
  strcpy(buf, ptr);
   
  // If we have the file send it.
  if ((ListSearch(buf) == 1)) {
    SendFile(buf, message->address);
    return;
  }
  // if in cache
  else if (CacheGetFile(buf, datagram, &size) == OK) {
    SendBuffer(buf, datagram, size, message->address);
    return;
  }
  // We do not have it but we know a place to get it   
  else {
    size = DirectoryGetListBySearch(buf, list);
    if (size > 0) {
      SendTry(buf, size, list, message->address);
      SendRequest(buf);
    }  
  }
}

void ParseData(struct Message *message)
{
  char buf[20], *packet;
  unsigned short len, buflen, ptr;
  FILE *file = NULL;

  packet = (char *)message->packet;
  strcpy(buf, packet + 1); ptr++;
  buflen = strlen(buf) + 1; ptr += buflen;
  len = packet[ptr] << 8 | (unsigned char)packet[ptr + 1]; ptr += 2;
  if (RdsRemove(buf) != OK) {
    return;
  }
  
  CacheAdd(buf, packet + ptr, len);

  file = fopen(buf, "w");
  if (file == NULL) {
    printf("Could not open file %s\n", buf);
    return;
  }
  fwrite(packet + ptr, sizeof(char), len, file);
  fclose(file);
  printf("Saved file %s\n", buf);
}

void ParseTry(struct Message *message)
{
  int i;
  char buf[20], *packet, ptr;
  unsigned short len, count, port;
  struct sockaddr_in peer;
  struct sockaddr_in addresses[20];
  
  packet = (char *)message->packet;
  strcpy(buf, packet + 2); ptr += 2;
  len = strlen(buf) + 1; ptr += len;
  count = ntohs( packet[ptr] << 8 | (unsigned char)packet[ptr+1]);
  ptr +=2;
  peer.sin_family = AF_INET;
  for (i = 0; i < count; i++) {
    memcpy(&peer.sin_addr, packet + ptr, 4); ptr += 4;
    port = packet[ptr] << 8 | (unsigned char)packet[ptr+1];
    peer.sin_port = htons(port);
    ptr += 2;
    addresses[i] = peer;
    DirectoryAdd(buf, peer);
  }
  if (RdsSearch(buf) == OK)
    SendRequestTo(buf, count, addresses);
}
