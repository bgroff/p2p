#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "sender.h"

static struct List list;
static struct ListingMessage message;
static int size;

void ListInit()
{
  list.count = 0;
}

/* Add an item to the list of files
 * Takes a string.
 * Returns nothing.
 */
void ListAdd(char * string)
{
  if (list.count < 56) {
    strcpy(list.list[list.count], string);
    list.count++;
  }
}

/* Return the number of elements in the list.
 * Takes no parameters.
 */
int ListCount()
{
  return list.count;
}

/* Return a populated ListingMessage
 * that is ready to be sent accross the
 * network. 
 *
 */
struct ListingMessage ListGetMessage()
{
  int i, length = 0;
  char buf[20*56];
  // Setup the message.
  message.cont = 0xCC;
  message.code = 0x4C;
  message.count = htons(list.count);
  
  // Add all the files.
  for (i = 0; i < list.count; i++) {
    strcpy(buf + length, list.list[i]);
    length += strlen(list.list[i]) + 1;
    buf[length - 1] = '\0';
  }
  
  message.entries = malloc(sizeof(char) * length);
  memcpy(message.entries, buf, length);
  size = length + 4;
  return message;
}

/* Return the size of the message 
 * that will be sent over the network.
 */
int ListMessageSize()
{
  return size;
}

/* Return the list of items.
 *
 */
struct List ListGet()
{
  return list;
}

int ListSearch(char *file)
{
  int i;
  for (i = 0; i < list.count; i++) {
    if (strcmp(file, list.list[i]) == 0)
      return 1;
  }
  return 0;
}
