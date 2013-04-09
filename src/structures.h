/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Data structures
 *
 */

#ifndef STRUCTURES_H_
#define STRUCTURES_H_


struct userInfo
{
        char nickname[9];
        char username[9];
        char name[50];
};

typedef struct userInfo userInfo;

#endif /* STRUCTURES_H_ */
