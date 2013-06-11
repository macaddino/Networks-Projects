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

#include "structures.h"

size_t user_info_size(const void *el);
int nick_comparator(const void *a, const void *b);
int seeker(const void *el, const void ** nick);
size_t chan_info_size(const void *el);
int chan_comparator(const void *a, const void *b);
int chan_seeker(const void *el, const void ** name);
size_t chanmode_info_size(const void *el);
int chanmode_comparator(const void *a, const void *b);
int chanmode_seeker(const void *el, const void ** name);

#endif /* LISTFXNS_H_ */
