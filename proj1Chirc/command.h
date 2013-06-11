/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Command codes
 *
 */
#ifndef COMMAND_H_
#define COMMAND_H_

#include "simclist.h"
#include "structures.h"

#define COMMANDNUM 19

#define NICK 	0
#define USER 	1
#define MOTD 	2
#define PRIVMSG 3
#define NOTICE  4
#define LUSERS	5
#define WHOIS 6
#define PING 7
#define PONG 8
#define QUIT 9
#define JOIN 10
#define PART 11
#define TOPIC 12
#define LIST 13
#define MODE 14
#define OPER 15
#define AWAY 16
#define NAMES 17
#define WHO 18

extern int num_pthreads;

extern pthread_mutex_t lock;

void command_init(char ** commands);
int command_search(char *command, char **comList);
void motd(userInfo *info, replyPackage * reply, serverInfo * servData);
int run_command(int command, char ** argList, int argNum, userInfo * info, list_t * userList, list_t * chanList, serverInfo * servData);
void user(char * username, char * name, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void nick(char * nickname, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void privmsg(char *to_nick, char *msg, userInfo *info, list_t *userList, list_t *chanList, replyPackage *reply, serverInfo *servData);
void notice(char *to_nick, char *msg, userInfo *info, list_t *userList, list_t *chanList, replyPackage *reply, serverInfo *servData);
void lusers(userInfo *info, list_t *userList, replyPackage *reply, serverInfo *servData);
void whois(char * nickname, userInfo * info, list_t * userList, replyPackage * reply, serverInfo * servData);
void ping(userInfo * info, serverInfo * servData);
void quit(char * msg, userInfo * info, list_t * userList, list_t * chanList);
void join(char * chanName, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void part(char * chanName, char * msg, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void topic(char * chanName, char * msg, userInfo * info, list_t * chanList, replyPackage * reply, serverInfo * servData);
void list(char * chanName, userInfo * info, list_t * chanList, replyPackage * reply, serverInfo * servData);
void mode(char * firstName, char * secondName, char * adjMode, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void oper(char * password, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void away(char * msg, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void names(char * chanName, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);
void who(char * mask, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData);

#endif /* COMMAND_H_ */
