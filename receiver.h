#ifndef RECEIVER_H_
#define RECEIVER_H_
#include <arpa/inet.h>

struct Message {
	struct sockaddr_in address;
	unsigned char *packet;
};
	
#define BUFLEN 1200
void ReceiverInit(int);
void * Receiver(void *);
void * ParseMessages(void *);
void ParseListing(struct Message*);
void ParseRequest(struct Message*);
void ParseData(struct Message*);
void ParseTry(struct Message*);

#endif
