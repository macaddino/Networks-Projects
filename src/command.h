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

#define COMMANDNUM 2

#define NICK 	0
#define USER 	1

void command_init(char ** commands);
int command_search(char *command, char **comList);
int run_command(int command, char ** argList, int argNum, userInfo * info, list_t * userList, int cliSocket, char * clientHost);
void user(char * username, char * name, userInfo * info, list_t * userList, int cliSocket, char * clientHost);
void nick(char * nickname, userInfo * info, list_t * userList, int cliSocket, char * clientHost);

#endif /* COMMAND_H_ */
