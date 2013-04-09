/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Data structures
 *
 */

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#define MAXNICK 10
#define MAXUSER 10
#define MAXNAME 51

struct userInfo
{
        char nickname[MAXNICK];
        char username[MAXUSER];
        char name[MAXNAME];
};

typedef struct userInfo userInfo;

#endif /* STRUCTURES_H_ */
