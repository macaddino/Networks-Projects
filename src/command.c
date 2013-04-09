/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Command Functions
 *
 */
#include <stdio.h>
#include <string.h>
#include "command.h"
#include "reply.h"
#include "structures.h"


/* command_init:
 * Initializes a list
 * of possible chIRC commands.
 */
void command_init(char ** commands)
{
        commands[0] = "NICK";
        commands[1] = "USER";
}


/* command_search:
 * Given a commad string and a list of commands,
 * outputs the index of the given command.
 * Returns -1 if given invalid command.
 */
int command_search(char *command, char **comList)
{
        for (int i=0; i<COMMANDNUM; i++)
        {
                if (!(strcmp(command, comList[i])))
                        return i;
        }
        return -1;
}


/* run_command:
 * Given a command, a list of client command arguments, a number of
 * arguments, a userInfo struct, a global list of users, a client socket,
 * and a client hostname, calls the appropriate function corresponding 
 * to the command sent by the client and passes on command arguments
 * and other required data to the appropriate function.
 * Returns 1 on success and -1 on failure.
 */
int run_command(int command, char ** argList, int argNum, userInfo * info, list_t * userList, int cliSocket, char * cliHost)
{
        switch(command)
        {
                case NICK:
                        if (argList[1])
                        {
                                nick(argList[1], info, userList, cliSocket, cliHost);
                                break;
                        }
                case USER:
                        if ((argList[1]) && (argList[4]))
                        {
                                user(argList[1], argList[4], info, userList, cliSocket, cliHost);
                                break;
                        }
                default  :
                        send_response(cliSocket, cliHost, ERR_UNKNOWNCOMMAND, info);
                        return -1;
        }
        return 1;
}


/* user:
 * Given a username, a name, a userInfo struct, a global list of users,
 * a client socket, and a client hostname, updates both local and global
 * user information, provided that the user has not previously registered.
 * Client responses:
 * ERR_ALREADYREGISTERED if user is already registered.
 * RPL_WELCOME if user data is entered for first time, but nickname
 *   data is already stored.
 */
void user(char * username, char * name, userInfo * info, list_t * userList, int cliSocket, char * cliHost)
{
        int userOverflow = 0;
        int nameOverflow = 0;
        int userLen = strlen(username);
        int nameLen = strlen(name);
        if (userLen >= MAXUSER)
        {
                userLen = MAXUSER;
                userOverflow = 1;
        }
        if (nameLen >= MAXNAME)
        {
                nameLen = MAXNAME;
                nameOverflow = 1;
        }
	// if both nickname and user data present, user is already registered
        if ((info->username[0]) && (info->nickname[0]))
        {
                send_response(cliSocket, cliHost, ERR_ALREADYREGISTRED, info);
        }
	// stores username and name locally
        else
        {
                memset(info->username, 0, MAXUSER);
                memset(info->name, 0, MAXNAME);
                int nameOffset = 0; // Accounts for offset of colon
		if (name[0] == ':')
			nameOffset = 1;
                for (int i=0; i<userLen; i++)
                {
                        info->username[i] = username[i];
                }
                if (userOverflow)
                        info->username[userLen-1] = '\0';
                for (int i=0; i<nameLen; i++)
                {
                        info->name[i] = name[i+nameOffset];
                }
                if (nameOverflow)
                        info->name[nameLen-1] = '\0';
		// stores user data globally if user has just registered
                if (info->nickname[0])
                {
                        list_append(userList, info);
                        // sort list
                        send_response(cliSocket, cliHost, RPL_WELCOME, info);
                        send_response(cliSocket, cliHost, RPL_YOURHOST, info);
                        send_response(cliSocket, cliHost, RPL_CREATED, info);
                        send_response(cliSocket, cliHost, RPL_MYINFO, info);
                }
        }
}


/* nick:
 * Given a nickname, a userInfo struct, a global list of users, a socket,
 * and a client hostname, locally and globally stores a user's nickname.
 * Client responses:
 * RPL_WELCOME if nick data is entered for first time, but user data
 *  is already stored.
 */
void nick(char * nickname, userInfo * info, list_t * userList, int cliSocket, char * cliHost)
{
        int nickLen = strlen(nickname);
        int nickOverflow = 0;
	int isFirstNick = 0;
        if (nickLen >= MAXNICK)
        {
                nickLen = MAXNICK;
                nickOverflow = 1;
        }
        if (!(info->nickname[0]))
                isFirstNick = 1;
        memset(info->nickname, 0, MAXNICK);
	// locally stores nickname
        for (int i=0; i<nickLen; i++)
        {
                info->nickname[i] = nickname[i];
        }
        if (nickOverflow)
                info->nickname[nickLen-1] = '\0';

	// globally stores user data if user has just registered
        if ((isFirstNick) && (info->username[0]))
        {
                list_append(userList, info);
                // now, sort the list based on nick
                send_response(cliSocket, cliHost, RPL_WELCOME, info);
                send_response(cliSocket, cliHost, RPL_YOURHOST, info);
                send_response(cliSocket, cliHost, RPL_CREATED, info);
                send_response(cliSocket, cliHost, RPL_MYINFO, info);
        }
	// globally updates nickname if user is already registered
        else if (!(isFirstNick))
        {
                printf("SimCList should be updated\n");
        }
}
