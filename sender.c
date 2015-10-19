#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include "p2p.h"
#include "rds.h"
#include "list.h"
#include "sender.h"
#include "directory.h"

extern int sock;

/* Every 240 seconds send out a message to all peers
 * with the files that we are serving. This is started 
 * on its own thread.
 * End when the program dies.
 */
void *SendListing(void *args)
{
  int i, len, ptr = 0;
  struct sockaddr_in dest[20];
  struct Packet packet;
  struct timespec sleep, counter;
  struct ListingMessage message = ListGetMessage();
  // Setup the packet
  packet.count = ListMessageSize();
  packet.data = malloc(sizeof(char) * packet.count);
  memcpy(packet.data, &(message.cont), 1); ptr++;
  memcpy(packet.data + ptr, &(message.code), 1); ptr++;
  memcpy(packet.data + ptr, &(message.count), 2); ptr += 2;
  memcpy(packet.data + ptr, message.entries, packet.count -4);
  
  // Setup the timers
  sleep.tv_sec = 240;
  sleep.tv_nsec = 0;
  while (1) {
    len = DirectoryGetList(dest);
    for (i = 0; i < len; i++) {
      Send(&dest[i], &packet);
    }
    nanosleep(&sleep, &counter);
  }
  free(packet.data);
}

/* Send a UDP Packet to the Destination.
 * dest is the destination ip/port.
 * packet is the packet you wish to send.
 */
int Send(struct sockaddr_in *dest, struct Packet *packet)
{
  socklen_t slen = sizeof(struct sockaddr_in);
  if (sendto(sock, packet->data, packet->count, 0,
  	 	(struct sockaddr*)dest, slen)==-1) {
    return -1;
  }
}

void SendRequest(char *request)
{
  struct sockaddr_in dest[20];
  struct Packet packet;
  int len = strlen(request) + 1, ptr = 0, i;
  unsigned char cont = 0xCC;
  unsigned char code = 0x52;
  packet.count = 2 + len;
  packet.data = malloc(packet.count);

  // Create the packet
  memcpy(packet.data, &cont, 1); ptr++;
  memcpy(packet.data + ptr, &code, 1); ptr++;
  memcpy(packet.data + ptr, request, len);

  RdsAdd(request);

  // Send to everyone that I know
  len = DirectoryGetList(dest);
  for (i = 0; i < len; i++) {
    Send(&dest[i], &packet);
  }
}

void SendRequestTo(char *request, short count, struct sockaddr_in *addresses)
{
  struct sockaddr_in dest[20];
  struct Packet packet;
  int len = strlen(request) + 1, ptr = 0, i;
  unsigned char cont = 0xCC;
  unsigned char code = 0x52;
  packet.count = 2 + len;
  packet.data = malloc(packet.count);
  
  // Create the packet
  memcpy(packet.data, &cont, 1); ptr++;
  memcpy(packet.data + ptr, &code, 1); ptr++;
  memcpy(packet.data + ptr, request, len);
  for (i = 0; i < count; i++) {
    Send(&addresses[i], &packet);
  }
}

void SendFile(char *str, struct sockaddr_in address)
{
  unsigned char cont = 0xDD; 
  struct Packet packet; 
  short buflen, size, ptr = 0;
  FILE *file = NULL;
  
  file = fopen(str, "r");
  if (file == NULL) {
    fprintf(stderr, "Could not open file %s\n", str);
    return;
  } 
  fseek (file , 0 , SEEK_END);
  size = ftell(file);
  rewind(file);
  buflen = strlen(str) + 1;
  packet.count = buflen + size + 3;
  packet.data = malloc(packet.count);

  memcpy(packet.data, &cont, 1); ptr++;
  memcpy(packet.data + ptr, str, buflen); ptr += buflen;
  ((char *)packet.data)[ptr] = size >> 8;
  ((char *)packet.data)[ptr+1] = (size & 0xFF) % 256; ptr += 2;
  fread(packet.data + ptr, sizeof(char), size, file);
  Send(&address, &packet);
  fclose(file);
  free(packet.data);
}

void SendBuffer(char *str, char *buffer, short size, struct sockaddr_in address)
{
  short buflen, ptr = 0;
  struct Packet packet;
  unsigned char cont = 0xDD;

  buflen = strlen(str) + 1;
  packet.count = buflen + size + 3;
  packet.data = malloc(packet.count);

  memcpy(packet.data, &cont, 1); ptr++;
  memcpy(packet.data + ptr, str, buflen); ptr += buflen;
  ((char *)packet.data)[ptr] = size >> 8;
  ((char *)packet.data)[ptr+1] = (size & 0xFF) % 256; ptr += 2;
  memcpy(packet.data + ptr, buffer, size);
  Send(&address, &packet); 
  free(packet.data);
}

void SendTry(char *file, short count, struct sockaddr_in *addresses, struct sockaddr_in dest) 
{
  int i;
  unsigned char *buf;
  unsigned char cont = 0xCC;
  unsigned char cond = 0x54; 
  struct Packet packet; 
  unsigned short buflen, ptr = 0, port;

  buflen = strlen(file) + 1;
  packet.count = 4 + buflen + (count * 6); // 4 bytes (cont, cond, count) + filename + each address
  packet.data = malloc(packet.count);
  
  memcpy(packet.data, &cont, 1); ptr++;
  memcpy(packet.data + ptr, &cond, 1); ptr++;
  memcpy(packet.data + ptr, file, buflen); ptr += buflen;
  memcpy(packet.data + ptr, &count, 2); ptr += 2;
  // Copy addresses
  for (i = 0; i < count; i++) {
    memcpy(packet.data + ptr, &(addresses[i].sin_addr), 4); ptr += 4;
    port = ntohs(addresses[i].sin_port);
    ((char *)packet.data)[ptr] = port >> 8;
    ((char *)packet.data)[ptr + 1] = (port & 0xFF) % 256;
	ptr += 2;
  }
  Send(&dest, &packet);
  free(packet.data);
}
