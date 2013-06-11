/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 * Aux Functions and global vars to create global lists maintained
 * throughout a server runtime.
 *
 */

#ifndef LISTFXNS_H_
#define LISTFXNS_H_

#include "transport.h"

size_t packet_data_size(const void *el);
int seq_comparator(const void *a, const void *b);
int seeker(const void *el, const void * key);

#endif /* LISTFXNS_H_ */
