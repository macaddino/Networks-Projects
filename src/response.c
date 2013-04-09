/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Server Response Functions
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "command.h"
#include "reply.h"
#include "structures.h"


/* send_response:
 * Given the client socket, the response code, and a userInfo struct,
 * sends an appropriate response to the client.
 * Returns 0 upon success.
 */
int send_response(int clientSocket, char * response, userInfo * info)
{
        char serverName[] = "sirius.cs.uchicago.edu";
        char clientHost[] = "sirius.cs.uchicago.edu";
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
        if (!(strcmp(response, RPL_WELCOME)))
        {
                snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, RPL_WELCOME, info->nickname);
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
                snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, RPL_YOURHOST, info->nickname);
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
                snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, RPL_CREATED, info->nickname);
                replyEndLen = strlen(RPL_CREATED_MSG) +
                              strlen(createdDate) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                snprintf(replyEnd, replyEndLen, RPL_CREATED_MSG, createdDate);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));
        }
        else if (!(strcmp(response, RPL_MYINFO)))
        {
                snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, RPL_MYINFO, info->nickname);
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
                // Handle case for when nick is not stored
                snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, ERR_UNKNOWNCOMMAND, info->nickname);
                replyEndLen = strlen(ERR_UNKNOWNCOMMAND_MSG) + 1;
                replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
                memcpy(replyEnd, ERR_UNKNOWNCOMMAND_MSG, replyEndLen);
                replyTotalLen = strlen(replyBeginning) + strlen(replyEnd) + 1;
                replyTotal = (char *) malloc(replyTotalLen*sizeof(char *));

        }
        else if (!(strcmp(response, ERR_ALREADYREGISTRED)))
        {
                snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", serverName, ERR_ALREADYREGISTRED, info->nickname);
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
        return 0;
}
