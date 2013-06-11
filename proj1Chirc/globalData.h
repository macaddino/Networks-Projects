/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 * Global variables pertaining to pthreads and mutexes.
 *
 *
 */

#ifndef GLOBALDATA_H_
#define GLOBALDATA_H_

pthread_mutex_t lock;
pthread_mutex_t chanLock;
int num_pthreads;

#endif /* GLOBALDATA_H_ */
