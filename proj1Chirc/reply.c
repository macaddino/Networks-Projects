/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Server Response Functions
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "globalData.h"
#include "reply.h"
#include "structures.h"


extern pthread_mutex_t lock;
extern int num_pthreads;


/* send_response:
 * Given the client socket, and a replyPackage struct
 * sends an appropriate response to the client.
 * Returns 1 upon success and -1 upon failure.
 */
int send_response(int clientSocket, replyPackage * reply)
{
  char **args = NULL;

  // unpack the replyStruct argument list
  // consider encompassing this into a new function
  if (reply->numArgs > 0)
  {
    int argIndx = 0;
    int argCharIndx = 0;
    args = (char **) malloc(reply->numArgs*sizeof(char **));
    for (int i = 0; i<=strlen(reply->args); i++)
    {
      if ((reply->args[i] == ' ') || (i == strlen(reply->args)))
      {
        args[argIndx] = (char *) malloc(argCharIndx*sizeof(char *));
        memcpy(args[argIndx], &(reply->args[i-argCharIndx]), argCharIndx);
        args[argIndx][argCharIndx] = '\0';
        argIndx++;
        argCharIndx = 0;
      }
      else
        argCharIndx++;
    }
  }  

  // create beginning of response message
  char *replyEnd;
  int replyEndLen;
  int replyBeginLen = 1 + strlen(reply->serverName) + // account for colon
                      strlen(reply->responseCode) + // reply identifier
                      strlen(reply->nickname) +
                      4; // account for spaces
  char replyBeginning[replyBeginLen];
  snprintf(replyBeginning, replyBeginLen, ":%s %s %s ", reply->serverName, 
                                                        reply->responseCode,
                                                        reply->nickname);
  // create end of response message based on appropriate response
  if (!(strcmp(reply->responseCode, RPL_WELCOME)))
  {
    replyEndLen = strlen(RPL_WELCOME_MSG) +
                  strlen(reply->nickname) +
                  strlen(args[0]) +
                  strlen(args[1]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_WELCOME_MSG, reply->nickname, args[0], args[1]);
  }
  else if (!(strcmp(reply->responseCode, RPL_YOURHOST)))
  {
    replyEndLen = strlen(RPL_YOURHOST_MSG) +
                  strlen(reply->serverName) +
                  strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_YOURHOST_MSG, reply->serverName, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_CREATED)))
  {
    replyEndLen = strlen(RPL_CREATED_MSG) +
                  strlen(reply->message);// + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_CREATED_MSG, reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_MYINFO)))
  {
    replyEndLen = strlen(RPL_MYINFO_MSG) +
                  strlen(reply->serverName) +
                  strlen(args[0]) +
                  strlen(args[1]) +
                  strlen(args[2]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_MYINFO_MSG, reply->serverName, args[0], args[1], args[2]);
  }
  else if (!(strcmp(reply->responseCode, ERR_UNKNOWNCOMMAND)))
  {
    replyEndLen = strlen(ERR_UNKNOWNCOMMAND_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, ERR_UNKNOWNCOMMAND_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, ERR_ALREADYREGISTRED)))
  {
    replyEndLen = strlen(ERR_ALREADYREGISTERED_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    memcpy(replyEnd, ERR_ALREADYREGISTERED_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, ERR_NICKNAMEINUSE)))
  {
    replyEndLen = strlen(args[0]) +
                  strlen(ERR_NICKNAMEINUSE_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, ERR_NICKNAMEINUSE_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_MOTDSTART)))
  {
    replyEndLen = strlen(reply->serverName) +
                  strlen(RPL_MOTDSTART_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_MOTDSTART_MSG, reply->serverName);
  }
  else if (!(strcmp(reply->responseCode, RPL_ENDOFMOTD)))
  {
    replyEndLen = strlen(RPL_ENDOFMOTD_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    memcpy(replyEnd, RPL_ENDOFMOTD_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, RPL_MOTD)))
  {
    replyEndLen = strlen(RPL_MOTD_MSG) + strlen(reply->message) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_MOTD_MSG, reply->message);
  }
  else if (!(strcmp(reply->responseCode, ERR_NOMOTD)))
  {
    replyEndLen = strlen(ERR_NOMOTD_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    memcpy(replyEnd, ERR_NOMOTD_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, ERR_NOSUCHNICK)))
  {
    replyEndLen = strlen(ERR_NOSUCHNICK_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, ERR_NOSUCHNICK_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_LUSERCLIENT)))
  {
    replyEndLen = strlen(RPL_LUSERCLIENT_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_LUSERCLIENT_MSG, args[5], args[1], args[6]);
  }
  else if (!(strcmp(reply->responseCode, RPL_LUSEROP)))
  {
    replyEndLen = strlen(RPL_LUSEROP_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_LUSEROP_MSG, args[2]);   
  }
  else if (!(strcmp(reply->responseCode, RPL_LUSERUNKNOWN)))
  {
    replyEndLen = strlen(RPL_LUSERUNKNOWN_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_LUSERUNKNOWN_MSG, args[4]);     
  }
  else if (!(strcmp(reply->responseCode, RPL_LUSERCHANNELS)))
  {
    replyEndLen = strlen(RPL_LUSERCHANNELS_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_LUSERCHANNELS_MSG, args[3]);     
  }
  else if (!(strcmp(reply->responseCode, RPL_LUSERME)))
  {
    replyEndLen = strlen(RPL_LUSERME_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_LUSERME_MSG, args[0], args[6]);   
  }
  else if (!(strcmp(reply->responseCode, RPL_WHOISUSER)))
  {
    replyEndLen = strlen(RPL_WHOISUSER_MSG) +
                  strlen(args[0]) + strlen(args[1]) +
                  strlen(args[2]) + strlen(reply->message) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_WHOISUSER_MSG, 
             args[0], args[1], args[2], reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_WHOISSERVER)))
  {
    replyEndLen = strlen(RPL_WHOISSERVER_MSG) +
                  strlen(args[0]) + strlen(args[1]) +
                  strlen(args[2]) +  1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_WHOISSERVER_MSG,
             args[0], args[1], args[2]);
  }
  else if (!(strcmp(reply->responseCode, RPL_ENDOFWHOIS)))
  {
    replyEndLen = strlen(RPL_ENDOFWHOIS_MSG) +
                  strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_ENDOFWHOIS_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_NAMREPLY)))
  {
    replyEndLen = strlen(RPL_NAMREPLY_MSG) + strlen(args[0]) +
                  strlen(args[1]) + strlen(reply->message) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_NAMREPLY_MSG, args[0], args[1], reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_ENDOFNAMES)))
  {
    replyEndLen = strlen(RPL_ENDOFNAMES_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_ENDOFNAMES_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, ERR_NOSUCHCHANNEL)))
  {
    replyEndLen = strlen(ERR_NOSUCHCHANNEL_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, ERR_NOSUCHCHANNEL_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, ERR_NOTONCHANNEL)))
  {
    replyEndLen = strlen(ERR_NOTONCHANNEL_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, ERR_NOTONCHANNEL_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, ERR_CANNOTSENDTOCHAN)))
  {
    replyEndLen = strlen(ERR_CANNOTSENDTOCHAN_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, ERR_CANNOTSENDTOCHAN_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_TOPIC)))
  {
    replyEndLen = strlen(RPL_TOPIC_MSG) + strlen(args[0]) + strlen(reply->message);
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_TOPIC_MSG, args[0], reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_NOTOPIC)))
  {
    replyEndLen = strlen(RPL_NOTOPIC_MSG) + strlen(args[0]);
    replyEnd = (char *) malloc(replyEndLen*sizeof(char *));
    snprintf(replyEnd, replyEndLen, RPL_NOTOPIC_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_LIST)))
  {
    replyEndLen = strlen(RPL_LIST_MSG) + strlen(args[0]) +
                  strlen(args[1]) + strlen(reply->message) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_LIST_MSG, args[0], args[1], reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_LISTEND)))
  {
    replyEndLen = strlen(RPL_LISTEND_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, RPL_LISTEND_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, RPL_WHOISCHANNELS)))
  {
    replyEndLen = strlen(RPL_WHOISCHANNELS_MSG) + strlen(args[0]) +
                  strlen(reply->message) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_WHOISCHANNELS_MSG, args[0], reply->message);
  }
  else if (!(strcmp(reply->responseCode, ERR_UMODEUNKNOWNFLAG)))
  {
    replyEndLen = strlen(ERR_UMODEUNKNOWNFLAG_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, ERR_UMODEUNKNOWNFLAG_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, ERR_USERSDONTMATCH)))
  {
    replyEndLen = strlen(ERR_USERSDONTMATCH_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, ERR_USERSDONTMATCH_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, ERR_UNKNOWNMODE)))
  {
    replyEndLen = strlen(ERR_UNKNOWNMODE_MSG) + strlen(args[0]) +
                  strlen(args[1]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, ERR_UNKNOWNMODE_MSG, args[0], args[1]);
  }
  else if (!(strcmp(reply->responseCode, RPL_CHANNELMODEIS)))
  {
    replyEndLen = strlen(RPL_CHANNELMODEIS_MSG) + strlen(args[0]) +
                  strlen(args[1]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_CHANNELMODEIS_MSG, args[0], args[1]);
  }
  else if (!(strcmp(reply->responseCode, ERR_CHANOPRIVSNEEDED)))
  {
    replyEndLen = strlen(ERR_CHANOPRIVSNEEDED_MSG) + strlen(args[0]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, ERR_CHANOPRIVSNEEDED_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, ERR_USERNOTINCHANNEL)))
  {
    replyEndLen = strlen(ERR_USERNOTINCHANNEL_MSG) + strlen(args[0]) +
                  strlen(args[1]) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, ERR_USERNOTINCHANNEL_MSG, args[0], args[1]);
  }
  else if (!(strcmp(reply->responseCode, ERR_PASSWDMISMATCH)))
  {
    replyEndLen = strlen(ERR_PASSWDMISMATCH_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, ERR_PASSWDMISMATCH_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, RPL_YOUREOPER)))
  {
    replyEndLen = strlen(RPL_YOUREOPER_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, RPL_YOUREOPER_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, RPL_NOWAWAY)))
  {
    replyEndLen = strlen(RPL_NOWAWAY_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, RPL_NOWAWAY_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, RPL_UNAWAY)))
  {
    replyEndLen = strlen(RPL_UNAWAY_MSG) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    memcpy(replyEnd, RPL_UNAWAY_MSG, replyEndLen);
  }
  else if (!(strcmp(reply->responseCode, RPL_AWAY)))
  {
    replyEndLen = strlen(RPL_AWAY_MSG) + strlen(args[0]) +
                  strlen(reply->message) + 1;
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_AWAY_MSG, args[0], reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_WHOREPLY)))
  {
    //printf("args is %s\n", reply->message);
    replyEndLen = strlen(reply->message);
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_WHOREPLY_MSG, reply->message);
  }
  else if (!(strcmp(reply->responseCode, RPL_ENDOFWHO)))
  {
    //printf("args is %s\n", reply->message);
    replyEndLen = strlen(args[0]) + strlen(RPL_ENDOFWHO_MSG);
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_ENDOFWHO_MSG, args[0]);
  }
  else if (!(strcmp(reply->responseCode, RPL_WHOISOPERATOR)))
  {
    //printf("args is %s\n", reply->message);
    replyEndLen = strlen(args[0]) + strlen(RPL_WHOISOPERATOR_MSG);
    replyEnd = (char *) malloc(replyEndLen*sizeof(char));
    snprintf(replyEnd, replyEndLen, RPL_WHOISOPERATOR_MSG, args[0]);
  }
  else
  {
    return -1;
  }
  // send message to client
  pthread_mutex_lock(&lock);
  send(clientSocket, replyBeginning, replyBeginLen, 0);
  send(clientSocket, replyEnd, replyEndLen, 0);
  send(clientSocket, "\r\n", 2,  0);
  free(args);
  pthread_mutex_unlock(&lock);
  return 1;
}
