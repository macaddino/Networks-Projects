/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Server Response Functions
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "reply.h"
#include "structures.h"


/* send_response:
 * Given the client socket, the client hostname, the response code,
 * and a userInfo struct, sends an appropriate response to the client.
 * Returns 1 upon success and -1 upon failure.
 */
int send_response(int clientSocket, char * clientHost, char * response, userInfo * info)
{
	// These are dummy variables, which will be replaced later
        char serverName[] = "sirius.cs.uchicago.edu";
        char serverVersion[] = "250";
        char createdDate[] = "4/9/2013";
        char userModes[] = "1";
        char chanModes[] = "1";

        char *replyEnd;
        char *replyTotal;
        int replyEndLen;
        int replyTotalLen;
        int replyBeginLen = 1 + strlen(serverName) + // account for colon
                            3 + // reply identifier
                            strlen(info->nickname) +
                            4; // account for spaces
        char replyBeginning[replyBeginLen];
	snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, response, info->nickname);
        if (!(strcmp(response, RPL_WELCOME)))
        {
                replyEndLen = strlen(RPL_WELCOME_MSG) +
                              strlen(info->nickname) +
                              strlen(info->username) +
                              strlen(clientHost) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                snprintf(replyEnd, replyEndLen, RPL_WELCOME_MSG, info->nickname, info->username, clientHost);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));
        }
        else if (!(strcmp(response, RPL_YOURHOST)))
        {
                replyEndLen = strlen(RPL_YOURHOST_MSG) +
                              strlen(serverName) +
                              strlen(serverVersion) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                snprintf(replyEnd, replyEndLen, RPL_YOURHOST_MSG, serverName, serverVersion);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));
        }
        else if (!(strcmp(response, RPL_CREATED)))
        {
                replyEndLen = strlen(RPL_CREATED_MSG) +
                              strlen(createdDate) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                snprintf(replyEnd, replyEndLen, RPL_CREATED_MSG, createdDate);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));
        }
        else if (!(strcmp(response, RPL_MYINFO)))
        {
                replyEndLen = strlen(RPL_MYINFO_MSG) +
                              strlen(serverName) +
                              strlen(serverVersion) +
                              strlen(userModes) +
                              strlen(chanModes) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                snprintf(replyEnd, replyEndLen, RPL_MYINFO_MSG, serverName, serverVersion, userModes, chanModes);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));
        }
        else if (!(strcmp(response, ERR_UNKNOWNCOMMAND)))
        {
                // Need to handle case for when nick is not stored
                replyEndLen = strlen(ERR_UNKNOWNCOMMAND_MSG) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                memcpy(replyEnd, ERR_UNKNOWNCOMMAND_MSG, replyEndLen);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));

        }
        else if (!(strcmp(response, ERR_ALREADYREGISTRED)))
        {
                replyEndLen = strlen(ERR_ALREADYREGISTERED_MSG) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                memcpy(replyEnd, ERR_ALREADYREGISTERED_MSG, replyEndLen);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));
        }
        else
        {
                return -1;
        }

        snprintf(replyTotal, replyTotalLen, "%s%s", replyBeginning, replyEnd);
        send(clientSocket, replyTotal, replyTotalLen, 0);
        send(clientSocket, "\r\n", 2,  0);
        return 1;
}
