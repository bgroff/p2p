#ifndef SENDER_H_
#define SENDER_H_
#include <netinet/in.h>
#include "list.h"
#include "p2p.h"

struct ListingMessage {
  char cont;
  char code;
  short count;
  char *entries;
};
  
int Send(struct sockaddr_in *, struct Packet *);
void *SendListing(void *args);
void SendRequest(char *);
void SendTry(char *, short, struct sockaddr_in *, struct sockaddr_in); 
void SendFile(char *, struct sockaddr_in);
void SendBuffer(char *, char *, short, struct sockaddr_in);

#endif
