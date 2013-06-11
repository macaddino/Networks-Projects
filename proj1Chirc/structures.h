/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Data structures
 *
 */

#include "simclist.h"

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#define MAXNICK 10
#define MAXUSER 10
#define MAXNAME 51
#define REPLYCODELEN 4
#define MAXHOST 64
#define MAXVERSION 10
#define MAXCREATED 25
#define MAXUSERMODES 8
#define MAXCHANMODES 10 
#define MAXARGS 256
#define MAXCHANNAME 10
#define MAXTOPIC 512
#define MAXUSERINCHAN 20
#define MAXCHANSPECMODE 2
#define MAXPASSWORD 21
#define MAXAWAY 512

struct userInfo
{
  char nickname[MAXNICK];
  char username[MAXUSER];
  char name[MAXNAME];
  char host[MAXHOST];
  char away[MAXAWAY];
  int socket;
  char modes[MAXUSERMODES];
  list_t * channelModes;
};

typedef struct userInfo userInfo;

struct serverInfo
{
  char serverHost[MAXHOST];
  char serverVersion[MAXVERSION];
  char createdDate[MAXCREATED];
  char userModes[MAXUSERMODES];
  char chanModes[MAXCHANMODES];
  char passwd[MAXPASSWORD];
};

typedef struct serverInfo serverInfo;

 struct workerArgs
{
  int socket;
  char * clientHost;
  list_t * userList;
  list_t * chanList;
  serverInfo * servData;
};

struct replyPackage
{
  int numArgs;
  char args[MAXARGS];
  int clientSocket;
  char nickname[MAXNICK];
  char serverName[50];
  char responseCode[REPLYCODELEN];
  char message[512];
};

typedef struct replyPackage replyPackage;

struct channelData
{
  char name[MAXCHANNAME];
  list_t * userList;
  char topic[MAXTOPIC];
  char modes[MAXCHANMODES];
  pthread_mutex_t chanUserLock;
};

typedef struct channelData channelData;

struct forChannel
{
  char channelName[MAXCHANNAME];
  char modes[MAXCHANSPECMODE];
  int numModes;
};

typedef struct forChannel forChannel;

#endif /* STRUCTURES_H_ */
