/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Command Functions
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "command.h"
#include "reply.h"
#include "response.h"
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
 * Given a command macro, a list of client command arguments, a number of
 * arguments, a userInfo struct, a userList, and a client socket,
 * calls the appropriate function corresponding to the command sent by the
 * client and passes on command arguments and other required data to the
 * appropriate function.
 * Returns 1 on success and -1 on failure.
 */
int run_command(int command, char ** argList, int argNum, userInfo * info, list_t * userList, int cliSocket)
{
        switch(command)
        {
                case NICK:
                        if (argList[1])
                        {
                                nick(argList[1], info, userList, cliSocket);
                                break;
                        }
                case USER:
                        if ((argList[1]) && (argList[4]))
                        {
                                user(argList[1], argList[4], info, userList, cliSocket);
                                break;
                        }
                default  :
                        send_response(cliSocket, ERR_UNKNOWNCOMMAND, info);
                        return -1;
        }
        return 1;
}


/* user:
 * Given a username, a name, a userInfo struct, a userList, and a client
 * socket, updates the user information as long as the user has not
 * previously registered.
 * -If both the nickname data and user data have already been entered,
 * user is already registered, and ALREADYREGISTERED error is sent to client.
 * -Else, user data is inserted in userInfo struct. If nick data is already
 * present, info struct is updated and a new entry is inserted in userList
 * and RPL_WELCOME is sent to  client. If nick data is not present,
 * user data is updated in info struct.
 */
void user(char * username, char * name, userInfo * info, list_t * userList, int cliSocket)
{
        int maxUserLen = 10;
        int maxNameLen = 51;
        int userOverflow = 0;
        int nameOverflow = 0;
        int userLen = strlen(username);
        int nameLen = strlen(name);
        if (userLen >= maxUserLen)
        {
                userLen = maxUserLen;
                userOverflow = 1;
        }
        if (nameLen >= maxNameLen)
        {
                nameLen = maxNameLen;
                nameOverflow = 1;
        }
        if ((info->username[0]) && (info->nickname[0]))
        {
                send_response(cliSocket, ERR_ALREADYREGISTRED, info);
        }
        else
        {
                memset(info->username, 0, maxUserLen);
                memset(info->name, 0, maxNameLen);
                int nameOffset = 0; // Accounts for offset of colon
                for (int i=0; i<userLen; i++)
                {
                        info->username[i] = username[i];
                }
                if (userOverflow)
                        info->username[userLen-1] = '\0';
                if (name[0] == ':')
                        nameOffset = 1;
                for (int i=0; i<nameLen; i++)
                {
                        info->name[i] = name[i+nameOffset]; // Do not include ':' in name
                }
                if (nameOverflow)
                        info->name[nameLen-1] = '\0';
                if (info->nickname[0])
                {
                        list_append(userList, info);
                        // sort list
                        send_response(cliSocket, RPL_WELCOME, info);
                        send_response(cliSocket, RPL_YOURHOST, info);
                        send_response(cliSocket, RPL_CREATED, info);
                        send_response(cliSocket, RPL_MYINFO, info);
                }
        }
}


/* nick:
 * Given a nickname, a userInfo struct, a userList, and a socket,
 * modifies a user's nickname and updates the userList accordingly.
 * -If user data has not yet been entered, userInfo struct is
 * updated, but userList entry is not added.
 * -If user data has been enetered,
 * and nick is being entered for the first time, updates userInfo struct and
 * inserts new entry into userList and sends RPL_WELCOME to client.
 * -If both nick and user data have already
 * been entered, updates the userInfo struct and updates the userList entry.
 * (DOES NOT DO THIS UPDATE YET.)
 */
void nick(char * nickname, userInfo * info, list_t * userList, int cliSocket)
{
        int maxNickLen = 10;
        int nickLen = strlen(nickname);
        int nickOverflow = 0;
        if (nickLen >= maxNickLen)
        {
                nickLen = maxNickLen;
                nickOverflow = 1;
        }
        int isFirstNick = 0;

        if (!(info->nickname[0]))
                isFirstNick = 1;
        memset(info->nickname, 0, maxNickLen);
        for (int i=0; i<nickLen; i++)
        {
                info->nickname[i] = nickname[i];
        }
        if (nickOverflow)
                info->nickname[nickLen-1] = '\0';

        if ((isFirstNick) && (info->username[0]))
        {
                list_append(userList, info);
                // now, sort the list based on nick
                send_response(cliSocket, RPL_WELCOME, info);
                send_response(cliSocket, RPL_YOURHOST, info);
                send_response(cliSocket, RPL_CREATED, info);
                send_response(cliSocket, RPL_MYINFO, info);
        }
        else if (!(isFirstNick))
        {
                printf("SimCList should be updated\n");
                // Later, update list
        }
}
