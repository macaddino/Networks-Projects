/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Aux Functions to create global lists maintained
 *  throughout a server runtime.
 *
 */
#include <stdio.h>
#include <string.h>
#include "listfxns.h"


/* packet_data_size:
 * Returns size of the packetData struct.
 */
size_t packet_data_size(const void *el)
{
  return sizeof(packetData);
}


/*
 * compare packets by seq number * 
 * this function compares two elements:
 * <0: a greater than b
 * 0: a equivalent to b
 * >0: b greater than a
 */
int seq_comparator(const void *a, const void *b)
{
  packetData *seqA = (packetData *) a;
  packetData *seqB = (packetData *) b;
  int sA, sB;
  sA = seqA->seqNum;
  sB = seqB->seqNum;
  return (sA < sB) - (sA > sB);
}


/* return "match" when the packet ackNum matches the key */
int seeker(const void *el, const void * key)
{
  /* let's assume el and key being always != NULL */
  const packetData *data = (packetData *) el;
  const signed int * intKey = key;
  if (data->ackNum == *intKey)
    return 1;
  return 0;
}
