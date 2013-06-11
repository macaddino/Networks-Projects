/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Command Functions
 *
 */
#include <dirent.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "command.h"
#include "globalData.h"
#include "globalUser.h"
#include "listfxns.h"
#include "reply.h"
#include "structures.h"


extern pthread_mutex_t lock;
extern int num_pthreads;


/* command_init:
 * Initializes a list
 * of possible chIRC commands.
 */
void command_init(char ** commands)
{
  commands[0] = "NICK";
  commands[1] = "USER";
  commands[2] = "MOTD";
  commands[3] = "PRIVMSG";
  commands[4] = "NOTICE";
  commands[5] = "LUSERS";
  commands[6] = "WHOIS";
  commands[7] = "PING";
  commands[8] = "PONG";
  commands[9] = "QUIT";
  commands[10] = "JOIN";
  commands[11] = "PART";
  commands[12] = "TOPIC";
  commands[13] = "LIST";
  commands[14] = "MODE";
  commands[15] = "OPER";
  commands[16] = "AWAY";
  commands[17] = "NAMES";
  commands[18] = "WHO";
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
     {
       return i;
     }
  }
  return -1;
}


/* run_command:
 * Given a command, a list of client command arguments, a number of
 * arguments, a userInfo struct, a global list of users, 
 * and a serverInfo struct, calls the appropriate
 * function corresponding to the command sent by the client and passes
 * on command arguments and other required data to the appropriate function.
 * Returns 1 on success and -1 on failure.
 */
int run_command(int command, char ** argList, int argNum, userInfo * info, list_t * userList, list_t * chanList, serverInfo * servData)
{
  replyPackage reply;
  memset(&reply, 0, sizeof(replyPackage));
  memcpy(reply.serverName, servData->serverHost, strlen(servData->serverHost));
  if (info->nickname[0])
    memcpy(reply.nickname, info->nickname, strlen(info->nickname));
  else
  {
    reply.nickname[0] = '*';
    reply.nickname[1] = '\0';
  }

  switch(command)
  {
    case NICK:
      if (argList[1])
      {
        nick(argList[1], info, userList, chanList, &reply, servData);
        break;
      }
      else
        break;
    case USER:
      if ((argList[1]) && (argList[4]))
      {
        user(argList[1], argList[4], info, userList, chanList, &reply, servData);
        break;
      }
      else
        break;
    case MOTD:
      motd(info, &reply, servData);
      break;
    case PRIVMSG:
      if(argList[1] && argList[2])
      {
        privmsg(argList[1], argList[2], info, userList, chanList, &reply, servData);
        break;
      }
      else
        break;
    case NOTICE:
      if(argList[1] && argList[2])
      {
        notice(argList[1], argList[2], info, userList, chanList, &reply, servData);
        break;
      }
      else
        break;
    case LUSERS:
      lusers(info, userList, &reply, servData);
      break;
    case WHOIS:
      if (argList[1])
      {
        whois(argList[1], info, userList, &reply, servData);
        break;
      }
      else
        break;
    case PING:
      ping(info, servData);
      break;
    case QUIT:
      if (argNum < 2)
        quit(NULL, info, userList, chanList);
      else
        quit(argList[1], info, userList, chanList);
      break;
    case PONG:
      break;
    case JOIN:
      if (argList[1])
      {
        join(argList[1], info, userList, chanList, &reply, servData);
      }
      break;
    case PART:
      if (argNum == 2)
        part(argList[1], NULL, info, userList, chanList, &reply, servData);
      else if (argNum == 3)
        part(argList[1], argList[2], info, userList, chanList, &reply, servData);
      break;
    case TOPIC:
      if (argList[1] && argList[2] && argNum == 3)
        topic(argList[1], argList[2], info, chanList, &reply, servData);
      else if (argList[1] && argNum == 2)
        topic(argList[1], NULL, info, chanList, &reply, servData);
      break;
    case LIST:
      if (argNum == 2)
        list(argList[1], info, chanList, &reply, servData);
      else
        list(NULL, info, chanList, &reply, servData);
      break;
    case MODE:
      if (argNum == 2)
        mode(argList[1], NULL, NULL, info, userList, chanList, &reply, servData);
      else if (argNum == 3)
        mode(argList[1], NULL, argList[2], info, userList, chanList, &reply, servData);
      else if (argNum == 4)
        mode(argList[1], argList[3], argList[2], info, userList, chanList, &reply, servData);
      break;
    case OPER:
      if (argNum == 3)
        oper(argList[2], info, userList, chanList, &reply, servData);
      break;
    case AWAY:
      if (argNum == 1)
        away(NULL, info, userList, chanList, &reply, servData);
      else if (argNum == 2)
        away(argList[1], info, userList, chanList, &reply, servData);
      break;
    case NAMES:
      if (argNum == 2)
        names(argList[1], info, userList, chanList, &reply, servData);
      else if (argNum == 1)
        names(NULL, info, userList, chanList, &reply, servData);
      break;
    case WHO:
      if (argNum == 2)
      {
        who(argList[1], info, userList, chanList, &reply, servData);
      }
      break;
    default  :
      return -1;
  }
  return 1;
}


/* user:
 * Given a username, a name, a userInfo struct, a global list of users,
 * a replyPackage struct, and a serverInfo struct, updates both local and global
 * user information, provided that the user has not previously registered.
 * Client responses:
 * ERR_ALREADYREGISTERED if user is already registered.
 * RPL_WELCOME if user data is entered for first time, but nickname
 * data is already stored.
 */
void user(char * username, char * name, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int userOverflow = 0;
  int nameOverflow = 0;
  int argLen = 0;
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
    memcpy(reply->responseCode, ERR_ALREADYREGISTRED, REPLYCODELEN);
    send_response(info->socket, reply);
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
      pthread_mutex_lock(&lock);
      info->channelModes = (list_t *) malloc(sizeof(list_t));
      list_init(info->channelModes);
      list_attributes_copy(info->channelModes, chanmode_info_size, 1);
      list_attributes_comparator(info->channelModes, chanmode_comparator);
      list_attributes_seeker(info->channelModes, (element_seeker)chanmode_seeker);
      pthread_mutex_unlock(&lock);

      pthread_mutex_lock (&lock);
      list_append(userList, info);
      list_sort(userList, -1);
      pthread_mutex_unlock(&lock);

      memcpy(reply->nickname, info->nickname, strlen(info->nickname));
      
      memcpy(reply->responseCode, RPL_WELCOME, REPLYCODELEN);
      reply->numArgs = 2;
      argLen = strlen(info->username) + strlen(info->host) + reply->numArgs;
      snprintf(reply->args, argLen, "%s %s", info->username, info->host);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      memset(reply->args, 0, MAXARGS);

      memcpy(reply->responseCode, RPL_YOURHOST, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(servData->serverVersion) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", servData->serverVersion);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      memset(reply->args, 0, MAXARGS);
      
      memcpy(reply->responseCode, RPL_CREATED, REPLYCODELEN);
      reply->numArgs = 0;
      memset(reply->message, 0, 512);
      memcpy(reply->message, servData->createdDate, strlen(servData->createdDate));
      reply->message[strlen(servData->createdDate)-3] = '\0';
      send_response(info->socket, reply);
      memcpy(reply->responseCode, RPL_MYINFO, REPLYCODELEN);
      reply->numArgs = 3;
      argLen = strlen(servData->serverVersion) + strlen(servData->userModes) + strlen(servData->chanModes) + reply->numArgs;
      snprintf(reply->args, argLen, "%s %s %s", servData->serverVersion, servData->userModes, servData->chanModes);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      memset(reply->args, 0, MAXARGS);

      lusers(info, userList, reply, servData);
      motd(info, reply, servData);
    }
  }
}


/* nick:
 * Given a nickname, a userInfo struct, a global list of users,
 * a replyPackage struct, and a serverInfo struct,
 * locally and globally stores a user's nickname.
 * Client responses:
 * RPL_WELCOME and accompanying messages if nick data is entered for
 * first time, but user data is already stored.
 * ERR_NICKNAMEINUSE if nick has already been taken.
 */
void nick(char * nickname, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int nickLen = strlen(nickname);
  int nickOverflow = 0;
	int isFirstNick = 0;
  int globalIndex = -1;
  char originalNick[MAXNICK];
  memcpy(originalNick, info->nickname, strlen(info->nickname));
  originalNick[strlen(info->nickname)] = '\0';

  int argLen = 0;
  if (nickLen >= MAXNICK)
  {
    nickLen = MAXNICK;
    nickOverflow = 1;
  }
  if (!(info->nickname[0]))
    isFirstNick = 1;
  else
  {
    // if user registered, fetch global info data
    if (info->username[0])
    {
      pthread_mutex_lock(&lock);
      globalIndex = list_locate(userList, info);
      pthread_mutex_unlock(&lock);
    }
  }
  // determines if nick is already taken
  pthread_mutex_lock(&lock);
  if (list_seek(userList, &nickname))
  {
    pthread_mutex_unlock(&lock);

    memcpy(reply->responseCode, ERR_NICKNAMEINUSE, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(nickname) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", nickname);
    send_response(info->socket, reply);
    return;
  }
  else
    pthread_mutex_unlock(&lock);
	// locally stores nickname
  memset(info->nickname, 0, MAXNICK);
  for (int i=0; i<nickLen; i++)
  {
    info->nickname[i] = nickname[i];
  }
  if (nickOverflow)
    info->nickname[nickLen-1] = '\0';

	// globally stores user data if user has just registered
  if ((isFirstNick) && (info->username[0]))
  {
    pthread_mutex_lock(&lock);
    info->channelModes = (list_t *) malloc(sizeof(list_t));
    list_init(info->channelModes);
    list_attributes_copy(info->channelModes, chanmode_info_size, 1);
    list_attributes_comparator(info->channelModes, chanmode_comparator);
    list_attributes_seeker(info->channelModes, (element_seeker)chanmode_seeker);
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&lock);
    list_append(userList, info);
		list_sort(userList, -1);
    pthread_mutex_unlock(&lock);

    memcpy(reply->nickname, info->nickname, strlen(info->nickname));
    
    memcpy(reply->responseCode, RPL_WELCOME, REPLYCODELEN);
    reply->numArgs = 2;
    argLen = strlen(info->username) + strlen(info->host) + reply->numArgs;
    snprintf(reply->args, argLen, "%s %s", info->username, info->host);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    memset(reply->args, 0, MAXARGS);

    memcpy(reply->responseCode, RPL_YOURHOST, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(servData->serverVersion) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", servData->serverVersion);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    memset(reply->args, 0, MAXARGS);

    memcpy(reply->responseCode, RPL_CREATED, REPLYCODELEN);
    reply->numArgs = 0;
    memset(reply->message, 0, 512);
    memcpy(reply->message, servData->createdDate, strlen(servData->createdDate));
    send_response(info->socket, reply);
  
    memcpy(reply->responseCode, RPL_MYINFO, REPLYCODELEN);
    reply->numArgs = 3;
    argLen = strlen(servData->serverVersion) + strlen(servData->userModes) + strlen(servData->chanModes) + reply->numArgs;
    snprintf(reply->args, argLen, "%s %s %s", servData->serverVersion, servData->userModes, servData->chanModes);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    memset(reply->args, 0, MAXARGS);
    
    lusers(info, userList, reply, servData);
    motd(info, reply, servData);
  }
	// globally updates nickname if user is already registered
  else if (globalIndex != -1)
  {
    int replyBeginLen = 1 + strlen(originalNick) + // account for colon
                        1 + strlen(info->username) + // account for bang
                        1 + strlen(info->host) + // account for @
                        2; // account for spaces
    char replyBeginning[replyBeginLen];
    snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", originalNick,
                                                          info->username,
                                                          info->host);
    int replyEndLen = strlen("NICK") +
                      strlen(info->nickname) + 3; // account for spaces and colon
    char replyEnd[replyEndLen];
    snprintf(replyEnd, replyEndLen, "NICK :%s", info->nickname);
    pthread_mutex_lock(&lock);
    list_iterator_start(info->channelModes);
    while (list_iterator_hasnext(info->channelModes))
    {
      forChannel * chanModes = (forChannel *) list_iterator_next(info->channelModes);
      pthread_mutex_unlock(&lock);
      char * chanName = chanModes->channelName;
      pthread_mutex_lock(&chanLock);
      channelData * channel = (channelData *) list_seek(chanList, &chanName);
      pthread_mutex_unlock(&chanLock);
      pthread_mutex_lock(&channel->chanUserLock);
      list_iterator_start(channel->userList);
      while (list_iterator_hasnext(channel->userList))
      {
        userInfo * user = (userInfo *) list_iterator_next(channel->userList);
        send(user->socket, replyBeginning, replyBeginLen, 0);
        send(user->socket, replyEnd, replyEndLen, 0);
        send(user->socket, "\r\n", 2, 0);
      }
      list_iterator_stop(channel->userList);
      pthread_mutex_unlock(&channel->chanUserLock);
      pthread_mutex_lock(&lock);
      int userIndex = list_locate(channel->userList, info);
      pthread_mutex_unlock(&lock);
      pthread_mutex_lock(&channel->chanUserLock);
      list_delete_at(channel->userList, userIndex);
      list_insert_at(channel->userList, info, userIndex);
      list_sort(channel->userList, -1);
      pthread_mutex_unlock(&channel->chanUserLock);
      pthread_mutex_lock(&lock);
    }
    list_iterator_stop(info->channelModes);
    pthread_mutex_unlock(&lock);
    memcpy(reply->nickname, nickname, strlen(nickname));
    pthread_mutex_lock(&lock);
    list_delete_at(userList, globalIndex);
    list_insert_at(userList, info, globalIndex);
    list_sort(userList, -1);
    pthread_mutex_unlock(&lock);
  }
}


/* motd:
 * Given a userInfo struct, a replyPackage struct, and a serverInfo
 * struct, returns the Message of the Day to the client.
 * Client responses:
 * RPL_MOTDSTART, RPL_MOTD, and, RPL_ENDOFMOTD if motd.txt found
 *  in directory chirc is run from.
 * ERR_NOMOTD if motd.txt not found.
 */
void motd(userInfo *info, replyPackage * reply, serverInfo * servData)
{
  FILE *f;
  f = fopen("motd.txt", "r");
  reply->clientSocket = info->socket;
  reply->numArgs = 0;

  memcpy(reply->nickname, info->nickname, strlen(info->nickname));
  memcpy(reply->serverName, servData->serverHost, strlen(servData->serverHost));
  memcpy(reply->responseCode, RPL_MOTDSTART, REPLYCODELEN);
  memset(reply->message, 0, 512);
  
  char line[512];
  memset(line, 0, 512);
  
  if (f == NULL)
  {
    memcpy(reply->responseCode, ERR_NOMOTD, REPLYCODELEN); 
    send_response(info->socket, reply);
  } 
  else
  {
    send_response(info->socket, reply);
    while(fgets(line, sizeof(char)*512, f))
    {
      memcpy(reply->responseCode, RPL_MOTD, REPLYCODELEN);
      memcpy(reply->message, line, strlen(line));
      // make sure final line of motd correctly NULL terminated
      // regardless of whether or not it ends with new line
      if (reply->message[strlen(line)] == '\n')
        reply->message[strlen(line) - 1] = '\0';
      else
        reply->message[strlen(line)] = '\0';
      send_response(info->socket, reply);
      memset(reply->message, 0, 512);
    }
    memcpy(reply->responseCode, RPL_ENDOFMOTD, REPLYCODELEN);
    send_response(info->socket, reply);
  }
  return;
}


/* privmsg:
 * Given a destination nickname, a message, userInfo struct, 
 * a global list of users, a replyPackage struct, and a serverInfo struct,
 * sends a private message from source user to destination user.
 * Client responses:
 * ERR_NOSUCHNICK if destination nickname is not valid.
 */
void privmsg(char *to_nick, char *msg, userInfo *info, list_t *userList, list_t *chanList, replyPackage *reply, serverInfo *servData)
{
  userInfo *recieving_user = malloc(sizeof(userInfo));
  memcpy(reply->nickname, info->nickname, strlen(info->nickname));
  memcpy(reply->serverName, servData->serverHost, strlen(servData->serverHost));
  memset(recieving_user, 0, sizeof(userInfo));
  reply->numArgs = 0;
  int argLen;

  // determine if message is for channel or nick
  if (to_nick[0] == '#')
  {
    // remove # from channel
    memcpy(to_nick, &to_nick[1], strlen(to_nick)-1);
    to_nick[strlen(to_nick)-1] = '\0';
    
    channelData * to_channel;
    // if channel does not exist
    pthread_mutex_lock(&chanLock);
    if (!(to_channel = (channelData *) list_seek(chanList, &to_nick)))
    {
      pthread_mutex_unlock(&chanLock);
      reply->clientSocket = info->socket;
      memcpy(reply->responseCode, ERR_NOSUCHNICK, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(to_nick) + reply->numArgs + 1; // account for #
      snprintf(reply->args, argLen, "#%s", to_nick);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      return;
    }
    else
    {
      pthread_mutex_unlock(&chanLock);
      int canChat = 1;
      int inChannel = -1;
      pthread_mutex_lock(&to_channel->chanUserLock);
      if ((inChannel = list_locate(info->channelModes, to_channel)) > -1)
      {
        if (to_channel->modes[0] == 'm' || to_channel->modes[1] == 'm')
        {
          forChannel * userChannel = (forChannel *) list_get_at(info->channelModes, inChannel);
          if (userChannel->modes[0] != 'o' && 
              userChannel->modes[0] != 'v' &&
              info->modes[0] != 'o' &&
              info->modes[1] != 'o')
          {
            canChat = 0;
          }
        }
      }
      pthread_mutex_unlock(&to_channel->chanUserLock);
      // check if user is part of channel
      if ((inChannel == -1) || (canChat == 0))
      {
        pthread_mutex_unlock(&lock);
        reply->clientSocket = info->socket;
        memcpy(reply->responseCode, ERR_CANNOTSENDTOCHAN, REPLYCODELEN);
        reply->numArgs = 1;
        argLen = strlen(to_nick) + reply->numArgs;
        snprintf(reply->args, argLen, "%s", to_nick);
        reply->args[argLen] = '\0';
        send_response(info->socket, reply);
        return;
      }
      int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                          1 + strlen(info->username) + // account for bang
                          1 + strlen(info->host) + // account for @
                          2; // account for spaces
      char replyBeginning[replyBeginLen];
      snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                            info->username,
                                                            info->host);
      int replyEndLen = strlen("PRIVMSG") +
                        strlen(to_channel->name) +
                        strlen(msg) + 3; // account for spaces
      char replyEnd[replyEndLen];
      snprintf(replyEnd, replyEndLen, "PRIVMSG #%s %s", to_channel->name, msg);
      pthread_mutex_lock(&to_channel->chanUserLock);
      list_iterator_start(to_channel->userList);
      while (list_iterator_hasnext(to_channel->userList))
      {
        recieving_user = (userInfo *) list_iterator_next(to_channel->userList);
        if (strcmp(recieving_user->nickname, info->nickname))
        {
          send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
          send(recieving_user->socket, replyEnd, replyEndLen, 0);
          send(recieving_user->socket, "\r\n", 2, 0);
        }
      }
      list_iterator_stop(to_channel->userList);
      pthread_mutex_unlock(&to_channel->chanUserLock);
    }
    return;
  }
  // determine if nickname is valid
  pthread_mutex_lock(&lock);
  if (!(recieving_user = (userInfo *) list_seek(userList, &to_nick)))
  {
    pthread_mutex_unlock(&lock);
    reply->clientSocket= info->socket;
    memcpy(reply->responseCode, ERR_NOSUCHNICK, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(to_nick) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", to_nick);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    return;
  }
  else
    pthread_mutex_unlock(&lock);

  // receive away message if receiver is away
  if (recieving_user->modes[0] == 'a' || recieving_user->modes[1] == 'a')
  {
    memcpy(reply->responseCode, RPL_AWAY, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(recieving_user->nickname) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", recieving_user->nickname);
    reply->args[argLen] = '\0';
    memset(reply->message, 0, 512);
    memcpy(reply->message, recieving_user->away, strlen(recieving_user->away));
    send_response(info->socket, reply);
  }
  // send message to destination user 
  int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                      1 + strlen(info->username) + // account for bang
                      1 + strlen(info->host) + // account for @
                      2; // account for spaces

  char replyBeginning[replyBeginLen];
  snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                        info->username,
                                                        info->host);
  int replyEndLen = strlen("PRIVMSG") + 
                    strlen(recieving_user->nickname) +
                    strlen(msg) + 2; // account for spaces

  char replyEnd[replyEndLen];
  snprintf(replyEnd, replyEndLen, "%s %s %s", "PRIVMSG",
                                              recieving_user->nickname,
                                              msg);
  pthread_mutex_lock(&lock);
  send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
  send(recieving_user->socket, replyEnd, replyEndLen, 0);
  send(recieving_user->socket, "\r\n", 2, 0);
  pthread_mutex_unlock(&lock);
  return;
}


/* notice:
 * Given a destination nickname, a message, userInfo struct,
 * a global list of users, a replyPackage struct, and a serverInfo struct,
 * sends a private message from source user to destination user.
 * Automatic client responses are NOT sent in response.
 */
void notice(char *to_nick, char *msg, userInfo *info, list_t *userList, list_t *chanList, replyPackage *reply, serverInfo *servData)
{
  userInfo *recieving_user = malloc(sizeof(userInfo));
  memcpy(reply->nickname, info->nickname, strlen(info->nickname));
  memcpy(reply->serverName, servData->serverHost, strlen(servData->serverHost));
  memset(recieving_user, 0, sizeof(userInfo));
  reply->numArgs = 0;

  // determine if message is for channel or nick
  if (to_nick[0] == '#')
  {
    // remove # from channel
    memcpy(to_nick, &to_nick[1], strlen(to_nick)-1);
    to_nick[strlen(to_nick)-1] = '\0';

    channelData *to_channel;
    // if channel does not exist
    pthread_mutex_lock(&chanLock);
    if (!(to_channel = (channelData *) list_seek(chanList, &to_nick)))
    {
      pthread_mutex_unlock(&chanLock);
      return;
    }
    else
    {
      pthread_mutex_unlock(&lock);
      int canChat = 1;
      int inChannel = -1;
      pthread_mutex_lock(&to_channel->chanUserLock);
      if ((inChannel = list_locate(info->channelModes, to_channel)) > -1)
      {
        if (to_channel->modes[0] == 'm' || to_channel->modes[1] == 'm')
        {
          forChannel * userChannel = (forChannel *) list_get_at(info->channelModes, inChannel);
        if (userChannel->modes[0] != 'o' &&
            userChannel->modes[0] != 'v' &&
            info->modes[0] != 'o' &&
            info->modes[1] != 'o')
          {
            canChat = 0;
          }
        }
      }
      pthread_mutex_unlock(&to_channel->chanUserLock);
      // check if user is part of channel
      if ((inChannel == -1) || (canChat == 0))
      {
        pthread_mutex_unlock(&lock);
        return;
      }
      int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                          1 + strlen(info->username) + // account for bang
                          1 + strlen(info->host) + // account for @
                          2; // account for spaces
      char replyBeginning[replyBeginLen];
      snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                            info->username,
                                                            info->host);
      int replyEndLen = strlen("NOTICE") +
                        strlen(to_channel->name) +
                        strlen(msg) + 3; // account for spaces
      char replyEnd[replyEndLen];
      snprintf(replyEnd, replyEndLen, "NOTICE #%s %s", to_channel->name, msg);
      pthread_mutex_lock(&lock);
      list_iterator_start(to_channel->userList);
      while(list_iterator_hasnext(to_channel->userList))
      {
        recieving_user = (userInfo *) list_iterator_next(to_channel->userList);
        if (strcmp(recieving_user->nickname, info->nickname))
        {
          send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
          send(recieving_user->socket, replyEnd, replyEndLen, 0);
          send(recieving_user->socket, "\r\n", 2, 0);
        }
      }
      list_iterator_stop(to_channel->userList);
      pthread_mutex_unlock(&lock);
    }
    return;
  }

  // determine if nickname is valid
  pthread_mutex_lock(&lock);
  if (!(recieving_user = (userInfo *) list_seek(userList, &to_nick)))
  {
    pthread_mutex_unlock(&lock);
    return;
  }
  else
    pthread_mutex_unlock(&lock);

  // send message to destination user
  int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                      1 + strlen(info->username) + // account for bang
                      1 + strlen(info->host) + // account for @
                      2; // account for spaces

  char replyBeginning[replyBeginLen];
  snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                        info->username,
                                                        info->host);
  int replyEndLen = strlen("NOTICE") +
                    strlen(recieving_user->nickname) +
                    strlen(msg) + 2; // account for spaces

  char replyEnd[replyEndLen];
  snprintf(replyEnd, replyEndLen, "%s %s %s", "NOTICE",
                                              recieving_user->nickname,
                                              msg);
  pthread_mutex_lock(&lock);
  send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
  send(recieving_user->socket, replyEnd, replyEndLen, 0);
  send(recieving_user->socket, "\r\n", 2, 0);
  free(recieving_user);
  pthread_mutex_unlock(&lock);
  return;
}


/* lusers:
 * Given a userInfo struct, a global list of users, a replyPackage
 * struct, and a serverInfo struct, returns information about
 * the server and clients on the server.
 * Client responses:
 * RPL_LUSERCLIENT, RPL_LUSEROP, RPL_LUSERUNKNOWN, RPL_LUSERCHANNELS,
 * RPL_LUSERME.
 */
void lusers(userInfo *info, list_t *userList, replyPackage *reply, serverInfo *servData)
{
  // dummy variables to be filled in later
  int num_invisible = 0;
  int num_operators = 0;
  int num_channels = 0;
  int num_servers = 1;

  pthread_mutex_lock(&lock);
  int num_clients = num_pthreads;
  int num_users = list_size(userList);
  int num_unknown = num_clients - num_users;
  pthread_mutex_unlock(&lock);

  reply->numArgs = 7;

  // pack all of lusers arguments into reply struct
  // should be set to total number of digits plus spaces in future
  int argLen = 14; 
  snprintf(reply->args, argLen, "%d %d %d %d %d %d %d", num_clients, 
                                                       num_invisible,
                                                       num_operators,
                                                       num_channels,
                                                       num_unknown,
                                                       num_users,
                                                       num_servers);
  reply->args[argLen] = '\0';
  memcpy(reply->responseCode, RPL_LUSERCLIENT, REPLYCODELEN);
  send_response(info->socket, reply);
  memcpy(reply->responseCode, RPL_LUSEROP, REPLYCODELEN);
  send_response(info->socket, reply);
  memcpy(reply->responseCode, RPL_LUSERUNKNOWN, REPLYCODELEN);
  send_response(info->socket, reply);
  memcpy(reply->responseCode, RPL_LUSERCHANNELS, REPLYCODELEN);
  send_response(info->socket, reply);
  memcpy(reply->responseCode, RPL_LUSERME, REPLYCODELEN);
  send_response(info->socket, reply);
}


/* whois:
 * Given a nickname, a userInfo struct, a global list of users, a client
 * hostname, a replyPackage struct, and a serverInfo struct,
 * returns the nickname, username, and hostname of desired user.
 * Client responses:
 * RPL_WHOISUSER, RPL_WHOISSERVER, RPL_ENDOFWHOIS if nickname is valid.
 * ERR_NOSUCHNICK if nickname is invalid.
 */
void whois(char * nickname, userInfo * info, list_t * userList, replyPackage * reply, serverInfo * servData)
{
  memcpy(reply->nickname, info->nickname, strlen(info->nickname));
  memcpy(reply->serverName, servData->serverHost, strlen(servData->serverHost));
  int argLen;

  userInfo * user = malloc(sizeof(userInfo));
  pthread_mutex_lock(&lock);
  // checks if nickname is invalid
  if (!(user = (userInfo *) list_seek(userList, &nickname)))
  {
    pthread_mutex_unlock(&lock);
    memcpy(reply->responseCode, ERR_NOSUCHNICK, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(nickname) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", nickname);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
  }
  else
  {
    pthread_mutex_unlock(&lock);
    memcpy(reply->responseCode, RPL_WHOISUSER, REPLYCODELEN);
    reply->numArgs = 3;
    argLen = strlen(user->nickname) + strlen(user->username) + 
             strlen(user->host) + reply->numArgs;
    snprintf(reply->args, argLen, "%s %s %s", user->nickname, 
                                              user->username,
                                              user->host);
    reply->args[argLen] = '\0';
    memcpy(reply->message, user->name, strlen(user->name));
    reply->message[strlen(user->name)-1] = '\0';
    send_response(info->socket, reply);
    pthread_mutex_lock(&lock);
    if (list_size(user->channelModes) > 0)
    {
      memcpy(reply->responseCode, RPL_WHOISCHANNELS, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(user->nickname) + reply->numArgs;
      memset(reply->args, 0, MAXARGS);
      snprintf(reply->args, argLen, "%s", user->nickname);
      reply->args[argLen] = '\0';
      memset(reply->message, 0, 512);
      int totalReplyLen = 0;
      list_iterator_start(user->channelModes);
      while (list_iterator_hasnext(user->channelModes))
      {
        forChannel * chanAndMode = (forChannel *) list_iterator_next(user->channelModes);
        pthread_mutex_unlock(&lock);
        if (chanAndMode->modes[0] == 'o')
        {
          memcpy(reply->message+totalReplyLen, "@", 1);
          totalReplyLen++;
        }
        else if (chanAndMode->modes[0] == 'v')
        {
          memcpy(reply->message+totalReplyLen, "+", 1);
          totalReplyLen++;
        }
        memcpy(reply->message+totalReplyLen, "#", 1);
        totalReplyLen++;
        memcpy(reply->message+totalReplyLen, chanAndMode->channelName, strlen(chanAndMode->channelName));
        totalReplyLen += strlen(chanAndMode->channelName) + 1; // account for space char
        reply->message[totalReplyLen-1] = ' ';
        pthread_mutex_lock(&lock);
      }
      list_iterator_stop(user->channelModes);
      pthread_mutex_unlock(&lock);
      reply->message[totalReplyLen] = '\0';
      send_response(info->socket, reply);
      pthread_mutex_lock(&lock); 
    }
    pthread_mutex_unlock(&lock);
    memcpy(reply->responseCode, RPL_WHOISSERVER, REPLYCODELEN);
    reply->numArgs = 3;
    argLen = strlen(user->nickname) + strlen(servData->serverHost) +
             strlen(servData->serverVersion) + reply->numArgs;
    snprintf(reply->args, argLen, "%s %s %s", user->nickname,
                                              servData->serverHost,
                                              servData->serverVersion);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    
    if (user->modes[0] == 'a' || user->modes[1] == 'a')
    {
      memcpy(reply->responseCode, RPL_AWAY, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(user->nickname) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", user->nickname);
      reply->args[argLen] = '\0';
      memset(reply->message, 0, 512);
      memcpy(reply->message, user->away, strlen(user->away));
      send_response(info->socket, reply);
    }
    if (user->modes[0] == 'o' || user->modes[1] == 'o')
    {
      memcpy(reply->responseCode, RPL_WHOISOPERATOR, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(user->nickname) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", user->nickname);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
    }

    memcpy(reply->responseCode, RPL_ENDOFWHOIS, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(user->nickname) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", user->nickname);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
  }
  return;
}


/* ping:
 * Given a userInfo struct and a serverInfo struct,
 * Sends a "PONG" response to the client, which
 * should not be responded to.
 */
void ping(userInfo * info, serverInfo * servData)
{
  char * reply;
  char pongMsg[] = "PONG ";
  int replyLen = strlen(pongMsg) + strlen(servData->serverHost) + 1;
  reply = (char *) malloc(replyLen*sizeof(char *));
  snprintf(reply, replyLen, "%s%s", pongMsg, servData->serverHost);
  pthread_mutex_lock(&lock);
  send(info->socket, reply, replyLen, 0);
  send(info->socket, "\r\n", 2, 0);
  pthread_mutex_unlock(&lock);
  free(reply);
  return;
}


void quit(char * msg, userInfo * info, list_t * userList, list_t * chanList)
{
  int replyLen;
  char * reply;
  char quitMsg[] = "Error :Closing Link: %s (%s)";
  if (!msg)
  {
    char stdQuitMsg[] = "Client Quit";
    msg = (char *) malloc(strlen(stdQuitMsg)*sizeof(char *));
    memcpy(msg, stdQuitMsg, strlen(stdQuitMsg));
  }
  if (msg[0] == ':')
  {
    memcpy(msg, &msg[1], strlen(msg)-1);
    msg[strlen(msg)-2] = '\0';
  }
  replyLen = strlen(quitMsg) + strlen(info->host) + strlen(msg) + 1;
  reply = (char *) malloc(replyLen*sizeof(char *));
  snprintf(reply, replyLen, quitMsg, info->host, msg);
  
  // remove user from global user list
  pthread_mutex_lock(&lock);
  num_pthreads--;
  int userIndex = list_locate(userList, info);
  list_delete_at(userList, userIndex);
  list_sort(userList, -1);
  pthread_mutex_unlock(&lock);

  pthread_mutex_lock(&lock);
  send(info->socket, reply, replyLen, 0);
  send(info->socket, "\r\n", 2, 0);
  pthread_mutex_unlock(&lock);

  int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                      1 + strlen(info->username) + // account for bang
                      1 + strlen(info->host) + // account for @
                      2; // account for spaces
  char replyBeginning[replyBeginLen];
  snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                        info->username,
                                                        info->host);
  int replyEndLen = strlen("QUIT") +
                    strlen(msg) + 3; // account for spaces and colon
  char replyEnd[replyEndLen];
  snprintf(replyEnd, replyEndLen, "QUIT :%s", msg);
  pthread_mutex_lock(&lock);
  list_iterator_start(info->channelModes);
  while (list_iterator_hasnext(info->channelModes))
  {
    forChannel * chanModes = (forChannel *) list_iterator_next(info->channelModes);
    char * chanName = chanModes->channelName;
    pthread_mutex_lock(&chanLock);
    channelData * channel = (channelData *) list_seek(chanList, &chanName);
    pthread_mutex_unlock(&chanLock);
    list_iterator_start(channel->userList);
    while (list_iterator_hasnext(channel->userList))
    {
      userInfo * user = (userInfo *) list_iterator_next(channel->userList);
      send(user->socket, replyBeginning, replyBeginLen, 0);
      send(user->socket, replyEnd, replyEndLen, 0);
      send(user->socket, "\r\n", 2, 0);
    }
    list_iterator_stop(channel->userList);
    int userIndex = list_locate(channel->userList, info);
    list_delete_at(channel->userList, userIndex);
    list_sort(channel->userList, -1);
  }
  list_iterator_stop(info->channelModes);
  pthread_mutex_unlock(&lock);
  shutdown(info->socket, 2);
  return;
}


void join(char * chanName, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  // remove # from chanName
  if (chanName[0] == '#')
  {
    memcpy(chanName, &chanName[1], strlen(chanName)-1);
    chanName[strlen(chanName)-1] = '\0';
  }
  // figure out when to return ERR_NOSUCHCHANNEL
  channelData * channel;
  // if chanName is not part of chanList, create new channelData structure
  pthread_mutex_lock(&chanLock);
  channel = list_seek(chanList, &chanName);
  pthread_mutex_unlock(&chanLock);

  // create new channel if channel does not exist
  if (channel == NULL)
  {
    channelData * newChannel = (channelData *) malloc(sizeof(channelData));
    memset(newChannel, 0, sizeof(channelData));
    memcpy(newChannel->name, chanName, strlen(chanName));
    pthread_mutex_init(&newChannel->chanUserLock, NULL); // this could be problem
    // remember to delete mutex upon channel deletion
    newChannel->userList = (list_t *) malloc(sizeof(list_t));
    list_init(newChannel->userList);
    list_attributes_copy(newChannel->userList, user_info_size, 1);
    list_attributes_comparator(newChannel->userList, nick_comparator);
    list_attributes_seeker(newChannel->userList, (element_seeker) seeker);
    pthread_mutex_lock(&lock);
    list_append(newChannel->userList, info);
    list_sort(newChannel->userList, -1);
    pthread_mutex_lock(&chanLock);
    list_append(chanList, newChannel);
    list_sort(chanList, 1);
    pthread_mutex_unlock(&chanLock);
    pthread_mutex_unlock(&lock);
    channel = newChannel;

    // user who created channel is operator
    //int originalIndex = list_locate(userList, info);
    forChannel * memberStatusMode = (forChannel *) malloc(sizeof(forChannel));
    memset(memberStatusMode, 0, sizeof(forChannel));
    memcpy(memberStatusMode->channelName, chanName, sizeof(chanName));
    memcpy(memberStatusMode->modes, "o\0", 2);
    pthread_mutex_lock(&lock);
    list_append(info->channelModes, memberStatusMode);
    list_sort(info->channelModes, -1);
    // update global userList
    int originalIndex = list_locate(userList, info);
    list_delete_at(userList, originalIndex);
    list_insert_at(userList, info, originalIndex);
    list_sort(userList, -1);
    pthread_mutex_unlock(&lock);
  }
  else
  {
    // if part of chanList, obtain channelData structure and delete from list (so it can be updated)
    // check to see if user is already part of channel
    pthread_mutex_lock(&lock);
    if (list_locate(channel->userList, info) > -1)
    {
      pthread_mutex_unlock(&lock);
      return;
    }
    pthread_mutex_unlock(&lock);

    // Update the userList of channel
    pthread_mutex_lock(&channel->chanUserLock);
    pthread_mutex_lock(&chanLock);
    int originalIndex = list_locate(chanList, channel);
    pthread_mutex_unlock(&chanLock);
    list_append(channel->userList, info);
    list_sort(channel->userList, -1);
    pthread_mutex_unlock(&channel->chanUserLock);

    // update user channel list
    originalIndex = list_locate(userList, info);
    forChannel * memberStatusMode = (forChannel *) malloc(sizeof(forChannel));
    memset(memberStatusMode, 0, sizeof(forChannel));
    memcpy(memberStatusMode->channelName, chanName, sizeof(chanName));
    memset(memberStatusMode->modes, 0, 2);
    pthread_mutex_lock(&lock);
    list_append(info->channelModes, memberStatusMode);
    list_sort(info->channelModes, -1);
    // update global userList
    list_delete_at(userList, originalIndex);
    list_insert_at(userList, info, originalIndex);
    list_sort(userList, -1);
    pthread_mutex_unlock(&lock);
  }
  // send initial JOIN message to all users in channel
  int replyLen = 1 + strlen(info->nickname) + // account for colon
                 1 + strlen(info->username) + // account for bang
                 1 + strlen(info->host) + // account for @
                 strlen("JOIN") +
                 1 + strlen(chanName) + // account for #
                 3; // account for spaces
  char initReply[replyLen];
  snprintf(initReply, replyLen, ":%s!%s@%s JOIN #%s", info->nickname,
                                                      info->username,
                                                      info->host,
                                                      chanName);
  pthread_mutex_lock(&channel->chanUserLock);
  list_iterator_start(channel->userList);
  while (list_iterator_hasnext(channel->userList))
  {
    userInfo * user = (userInfo *) list_iterator_next(channel->userList);
    send(user->socket, initReply, replyLen, 0);
    send(user->socket, "\r\n", 2, 0);
  }
  list_iterator_stop(channel->userList);
  pthread_mutex_unlock(&channel->chanUserLock);

  if (channel->topic[0])
  {
    // If TOPIC present in channel, send RPL_TOPIC response
    topic(channel->name, NULL, info, chanList, reply, servData);
  }
  // send RPL_NAMREPLY response
  names(chanName, info, userList, chanList, reply, servData);
  return;
}

void part(char * chanName, char * msg, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int userIndex;
  int argLen;
  // remove # from chanName
  if (chanName[0] == '#')
  {
    memcpy(chanName, &chanName[1], strlen(chanName)-1);
    chanName[strlen(chanName)-1] = '\0';
  }
  // if channel doesn't exist, return ERR_NOSUCHCHANNEL
  channelData * channel;
  pthread_mutex_lock(&chanLock);
  channel = list_seek(chanList, &chanName);
  pthread_mutex_unlock(&chanLock);
  if (channel == NULL)
  {
    memcpy(reply->responseCode, ERR_NOSUCHCHANNEL, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(chanName) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", chanName);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    return;
  }
  // if user is not member of channel, return ERR_NOTONCHANNEL
  pthread_mutex_lock(&channel->chanUserLock);
  if ((userIndex = list_locate(channel->userList, info)) == -1)
  {
    pthread_mutex_unlock(&channel->chanUserLock);
    memcpy(reply->responseCode, ERR_NOTONCHANNEL, REPLYCODELEN);
    reply->numArgs = 1;
    argLen = strlen(chanName) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", chanName);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
    return;
  }
  pthread_mutex_unlock(&channel->chanUserLock);
  pthread_mutex_lock(&lock);
  // if user is op or voice in channel, delete this data from userList

  forChannel * chanAndModeRef;
  int originalIndex;
  if ((chanAndModeRef = list_seek(info->channelModes, &chanName)))
  {
    originalIndex = list_locate(info->channelModes, chanAndModeRef);
    list_delete_at(info->channelModes, originalIndex);
    originalIndex = list_locate(userList, info);
    //list_delete_at(userList, originalIndex);
    list_insert_at(userList, info, originalIndex);
    list_sort(userList, -1);
  }
  pthread_mutex_unlock(&lock);

  // alert all users in channel of the PART
  int replyLen = 1 + strlen(info->nickname) + // account for colon
                 1 + strlen(info->username) + // account for bang
                 1 + strlen(info->host) + // account for @
                 strlen("PART") +
                 1 + strlen(chanName) + // account for #
                 3; // account for spaces
  char initReply[replyLen];
  snprintf(initReply, replyLen, ":%s!%s@%s PART #%s", info->nickname,
                                                        info->username,
                                                        info->host,
                                                        chanName);
  int messageLen = 0;
  char * messageReply;
  // if msg isn't NULL, append message msg, beginning with :
  int msgPresent = 0;
  if (msg)
    msgPresent = 1;
  if (msgPresent)
  {
    // remove colon
    if (msg[0] == ':')
    {
      memcpy(msg, &msg[1], strlen(msg)-1);
      msg[strlen(msg)-2] = '\0';
    }
    messageLen = strlen(msg) + 3; // account for space
    pthread_mutex_lock(&channel->chanUserLock);
    messageReply = (char *) malloc(messageLen*sizeof(char));
    snprintf(messageReply, messageLen, " :%s", msg); // account for colon
    pthread_mutex_unlock(&channel->chanUserLock);
  }

  pthread_mutex_lock(&channel->chanUserLock);
  list_iterator_start(channel->userList);
  while (list_iterator_hasnext(channel->userList))
  {
    userInfo * user = (userInfo *) list_iterator_next(channel->userList);
    send(user->socket, initReply, replyLen, 0);
    if (msgPresent)
    {
      send(user->socket, messageReply, messageLen, 0);
    }
    send(user->socket, "\r\n", 2, 0);
  }
  list_iterator_stop(channel->userList);

  // remove user from channel userList
  list_delete_at(channel->userList, userIndex);
  pthread_mutex_unlock(&channel->chanUserLock);
  
  pthread_mutex_lock(&channel->chanUserLock);
  // if numUsers is 0, remove channel from chanList
  if (list_size(channel->userList) == 0)
  {
    pthread_mutex_unlock(&channel->chanUserLock);
    // delete userList here
    pthread_mutex_lock(&chanLock);
    int chanIndex = list_locate(chanList, channel);
    list_delete_at(chanList, chanIndex);
    pthread_mutex_unlock(&chanLock);
  }
  else
    pthread_mutex_unlock(&channel->chanUserLock);
  return;
}

void topic(char * chanName, char * msg, userInfo * info, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  // remove # from channel
  if (chanName[0] == '#')
  {
    memcpy(chanName, &chanName[1], strlen(chanName)-1);
    chanName[strlen(chanName)-1] = '\0';
  }
  channelData * channel;

  int argLen;

  // determine if user is on channel
  pthread_mutex_lock(&chanLock);
  if (!(channel = (channelData *) list_seek(chanList, &chanName)))
  {
    pthread_mutex_unlock(&chanLock);
    reply->numArgs = 1;
    argLen = strlen(chanName) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", chanName);
    reply->args[argLen] = '\0';
    memcpy(reply->responseCode, ERR_NOTONCHANNEL, REPLYCODELEN);
    send_response(info->socket, reply);
    return;
  }
  else
    pthread_mutex_unlock(&chanLock);
  pthread_mutex_lock(&lock);
  if (list_locate(channel->userList, info) == -1)
  {
    pthread_mutex_unlock(&lock);
    reply->numArgs = 1;
    argLen = strlen(channel->name) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", channel->name);
    reply->args[argLen] = '\0';
    memcpy(reply->responseCode, ERR_NOTONCHANNEL, REPLYCODELEN);
    send_response(info->socket, reply);
    return;
  }
  pthread_mutex_unlock(&lock);

  // display the topic
  if (msg == NULL)
  {
    // check for no topic
    if (!channel->topic[0])
    {
      reply->numArgs = 1;
      argLen = strlen(channel->name) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", channel->name);
      reply->args[argLen] = '\0';
      memcpy(reply->responseCode, RPL_NOTOPIC, REPLYCODELEN);
      send_response(info->socket, reply);
      return;
    }
    reply->numArgs = 1;
    argLen = strlen(channel->name) + reply->numArgs;
    snprintf(reply->args, argLen, "%s", channel->name);
    reply->args[argLen] = '\0';
    memcpy(reply->message, channel->topic, strlen(channel->topic));
    memcpy(reply->responseCode, RPL_TOPIC, REPLYCODELEN);
    send_response(info->socket, reply);
    return;
  }
  else
  {
    // if chan in topic mode, only op can change topic
    if (channel->modes[0] == 't' || channel->modes[1] == 't')
    {
      pthread_mutex_lock(&channel->chanUserLock);
      int chanIndex = list_locate(info->channelModes, channel);
      forChannel * userChannel = (forChannel *) list_get_at(info->channelModes, chanIndex);
      pthread_mutex_unlock(&channel->chanUserLock);
      if (userChannel->modes[0] != 'o' && 
          info->modes[0] != 'o' &&
          info->modes[1] != 'o')
      {
        memcpy(reply->responseCode, ERR_CHANOPRIVSNEEDED, REPLYCODELEN);
        reply->numArgs = 1;
        argLen = strlen(channel->name) + reply->numArgs;
        snprintf(reply->args, argLen, "%s", channel->name);
        reply->args[argLen] = '\0';
        send_response(info->socket, reply);
        return;
      }
    }
    // remove colon from message
    if (msg[0] == ':')
    {
      memcpy(msg, &msg[1], strlen(msg)-1);
      msg[strlen(msg)-2] = '\0';
    }
    // clear the topic if user sends only colon
    if (!strlen(msg))
    {
      memset(channel->topic, 0, MAXTOPIC);
    }
    // reset the topic
    else
    {
      memset(channel->topic, 0, MAXTOPIC);
      memcpy(channel->topic, msg, strlen(msg));
    }
    int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                        1 + strlen(info->username) + // account for bang
                        1 + strlen(info->host) + // account for @
                        2; // account for spaces
    char replyBeginning[replyBeginLen];
    snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                          info->username,
                                                          info->host);
    int replyEndLen = strlen("TOPIC") +
                      strlen(channel->name) +
                      strlen(msg) + 5; // account for colon, #, and spaces
    char replyEnd[replyEndLen];
    snprintf(replyEnd, replyEndLen, "TOPIC #%s :%s", channel->name, msg);

    userInfo * recieving_user;
    pthread_mutex_lock(&channel->chanUserLock);
    list_iterator_start(channel->userList);
    while (list_iterator_hasnext(channel->userList))
    {
      recieving_user = (userInfo *) list_iterator_next(channel->userList);
      send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
      send(recieving_user->socket, replyEnd, replyEndLen, 0);
      send(recieving_user->socket, "\r\n", 2, 0);
    }
    list_iterator_stop(channel->userList);
    pthread_mutex_unlock(&channel->chanUserLock);
  }
  return;
}


void list(char * chanName, userInfo * info, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int argLen;
  channelData * channel;
  int num_users;

  memcpy(reply->responseCode, RPL_LIST, REPLYCODELEN);
  reply->numArgs = 3;
  if (chanName == NULL)
  {
    pthread_mutex_lock(&chanLock);
    list_iterator_start(chanList);
    while (list_iterator_hasnext(chanList))
    {
      channel = (channelData *) list_iterator_next(chanList);
      pthread_mutex_unlock(&chanLock);
      if (strcmp(channel->name, "*"))
      {
        pthread_mutex_lock(&channel->chanUserLock);
        num_users = list_size(channel->userList);
        pthread_mutex_unlock(&channel->chanUserLock);
        int num_users_digits = 0;
        for (int i = num_users; i > 9; i= i/10)
          num_users_digits++;
        argLen = sizeof(channel->name) + num_users_digits + 4; // account for # and spaces
        memset(reply->args, 0, MAXARGS);
        snprintf(reply->args, argLen, "#%s %d ", channel->name, num_users);
        reply->args[argLen] = '\0';
        memset(reply->message, 0, 512);
        memcpy(reply->message, channel->topic, strlen(channel->topic));
        send_response(info->socket, reply);
      }
      pthread_mutex_lock(&chanLock);
    }
    list_iterator_stop(chanList);
    pthread_mutex_unlock(&chanLock);
  }
  else
  {
    // remove # from channel name
    if (chanName[0] == '#')
    {
      memcpy(chanName, &chanName[1], strlen(chanName)-1);
      chanName[strlen(chanName)-1] = '\0';
    }
    pthread_mutex_lock(&chanLock);
    if ((channel = (channelData *) list_seek(chanList, &chanName)))
    {
      pthread_mutex_lock(&channel->chanUserLock);
      num_users = list_size(channel->userList);
      pthread_mutex_unlock(&channel->chanUserLock);
      int num_users_digits = 0;
      for (int i = num_users; i > 9; i= i/10)
        num_users_digits++;
      argLen = sizeof(channel->name) + num_users_digits + 4; // account for # and spaces
      snprintf(reply->args, argLen, "#%s %d ", channel->name, num_users);
      reply->args[argLen] = '\0';
      memset(reply->message, 0, 512);
      memcpy(reply->message, channel->topic, strlen(channel->topic));
      send_response(info->socket, reply);
    }
    pthread_mutex_unlock(&chanLock);
  }
  memcpy(reply->responseCode, RPL_LISTEND, REPLYCODELEN);
  reply->numArgs = 0;
  send_response(info->socket, reply);
  return;
}


void mode(char * firstName, char * secondName, char * adjMode, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int argLen;
  // If we're dealing with a channel
  // remove # from channel name
  if (firstName[0] == '#')
  {
    memcpy(firstName, &firstName[1], strlen(firstName)-1);
    firstName[strlen(firstName)-1] = '\0';
    
    channelData * channel;
    pthread_mutex_lock(&chanLock);
    if (!(channel = (channelData *) list_seek(chanList, &firstName)))
    {
      pthread_mutex_unlock(&chanLock);
      // return an error, there's no such channel
      memcpy(reply->responseCode, ERR_NOSUCHCHANNEL, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(firstName) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", firstName);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      return;
    }
    pthread_mutex_unlock(&chanLock);
    // return mode of channel
    if (adjMode == NULL)
    {
      memcpy(reply->responseCode, RPL_CHANNELMODEIS, REPLYCODELEN);
      reply->numArgs = 2;
      argLen = strlen(channel->name) + strlen(channel->modes) + reply->numArgs;
      snprintf(reply->args, argLen, "%s %s", channel->name, channel->modes);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      return;
    }
    // make sure user is operator on channel
    userInfo * updatingUser;
    int globalIndex;
    pthread_mutex_lock(&channel->chanUserLock);
    if (list_locate(channel->userList, info) == -1 &&
        info->modes[0] != 'o' &&
        info->modes[1] != 'o')
    {
      pthread_mutex_unlock(&channel->chanUserLock);
      // person isn't on channel, can't make changes
      memcpy(reply->responseCode, ERR_CHANOPRIVSNEEDED, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(channel->name) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", channel->name);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      return;
    }
    updatingUser = info;
    forChannel * userChannel;
    globalIndex = list_locate(updatingUser->channelModes, channel);
    userChannel = (forChannel *) list_get_at(updatingUser->channelModes, globalIndex);
    pthread_mutex_unlock(&channel->chanUserLock);
    if (userChannel->modes[0] != 'o' && 
        info->modes[0] != 'o' && 
        info->modes[1] != 'o')
    {
      // user can't make updates, they're not an operator
      memcpy(reply->responseCode, ERR_CHANOPRIVSNEEDED, REPLYCODELEN);
      reply->numArgs = 1;
      argLen = strlen(channel->name) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", channel->name);
      reply->args[argLen] = '\0';
      send_response(info->socket, reply);
      return;
    }
    if (secondName == NULL)
    {
      if (adjMode[0] == '+')
      {
        if (adjMode[1] != 't' && adjMode[1] != 'm')
        {
          // return an error
          // incorrect flags
          memcpy(reply->responseCode, ERR_UNKNOWNMODE, REPLYCODELEN);
          reply->numArgs = 2;
          argLen = 1 + strlen(channel->name) + reply->numArgs;
          snprintf(reply->args, argLen, "%c %s", adjMode[1], channel->name);
          reply->args[argLen] = '\0';
          send_response(info->socket, reply);
          return;
        }
        // if either spot already contains that flag
        if (channel->modes[0] == adjMode[1] || channel->modes[1] == adjMode[1])
        {
          // send confirmation
        }
        // if the first spot is taken, set the second spot to the flag
        // otherwise, set the first spot to the flag
        if (channel->modes[0])
        {
          channel->modes[1] = adjMode[1];
          channel->modes[2] = '\0';
        }
        else
        {
          channel->modes[0] = adjMode[1];
          channel->modes[1] = '\0';
        }
        // send confirmation
        // return
        int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                            1 + strlen(info->username) + // account for bang
                            1 + strlen(info->host) + // account for @
                            2; // account for spaces
        char replyBeginning[replyBeginLen];
        snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                              info->username,
                                                              info->host);
        int replyEndLen = strlen("MODE") +
                          strlen(channel->name) +
                          strlen(adjMode) + 4; // account for # and spaces
        char replyEnd[replyEndLen];
        snprintf(replyEnd, replyEndLen, "MODE #%s %s", channel->name, adjMode);

        userInfo * recieving_user;
        pthread_mutex_lock(&channel->chanUserLock);
        list_iterator_start(channel->userList);
        while (list_iterator_hasnext(channel->userList))
        {
          recieving_user = (userInfo *) list_iterator_next(channel->userList);
          send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
          send(recieving_user->socket, replyEnd, replyEndLen, 0);
          send(recieving_user->socket, "\r\n", 2, 0);
        }
        list_iterator_stop(channel->userList);
        pthread_mutex_unlock(&channel->chanUserLock);
        return;
      }
      else if (adjMode[0] == '-')
      {
        if (adjMode[1] != 't' && adjMode[1] != 'm')
        {
          // return an error
          // incorrect flags
          memcpy(reply->responseCode, ERR_UNKNOWNMODE, REPLYCODELEN);
          reply->numArgs = 2;
          argLen = 1 + strlen(channel->name) + reply->numArgs;
          snprintf(reply->args, argLen, "%c %s", adjMode[1], channel->name);
          reply->args[argLen] = '\0';
          send_response(info->socket, reply);
          return;
        }
        // if the first spot has the flag
        if (channel->modes[0] == adjMode[1])
        {
          // move the second flag to the first position, set the second flag to NULL
          if (channel->modes[1] != '\0')
          {
            channel->modes[0] = channel->modes[1];
            channel->modes[1] = '\0';
          }
          else
            memset(&channel->modes[0], 0, 1);
          // send confirmation
        }
        else if (channel->modes[1] == adjMode[1])
        {
          // if the second flag matches, set that flag to NULL
          channel->modes[1] = '\0';
        }
        else
        {
          // that flag doesn't exist
        }
        int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                            1 + strlen(info->username) + // account for bang
                            1 + strlen(info->host) + // account for @
                            2; // account for spaces
        char replyBeginning[replyBeginLen];
        snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                              info->username,
                                                              info->host);
        int replyEndLen = strlen("MODE") +
                          strlen(channel->name) +
                          strlen(adjMode) + 4; // account for # and spaces
        char replyEnd[replyEndLen];
        snprintf(replyEnd, replyEndLen, "MODE #%s %s", channel->name, adjMode);

        userInfo * recieving_user;
        pthread_mutex_lock(&channel->chanUserLock);
        list_iterator_start(channel->userList);
        while (list_iterator_hasnext(channel->userList))
        {
          recieving_user = (userInfo *) list_iterator_next(channel->userList);
          send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
          send(recieving_user->socket, replyEnd, replyEndLen, 0);
          send(recieving_user->socket, "\r\n", 2, 0);
        }
        list_iterator_stop(channel->userList);
        pthread_mutex_unlock(&channel->chanUserLock);
      }
    }
    // end change channel mode
    // if the mode is for a channel specific user
    else
    {
      userInfo * updatingUser;
      pthread_mutex_lock(&channel->chanUserLock);
      if (!(updatingUser = (userInfo *) list_seek(channel->userList, &secondName)))
      {
        pthread_mutex_unlock(&channel->chanUserLock);
        memcpy(reply->responseCode, ERR_USERNOTINCHANNEL, REPLYCODELEN);
        reply->numArgs = 2;
        argLen = strlen(secondName) + strlen(channel->name) + reply->numArgs;
        snprintf(reply->args, argLen, "%s %s", secondName, channel->name);
        reply->args[argLen] = '\0';
        send_response(info->socket, reply);
        return;
        // return an error, there's no such user
      }
      forChannel * userChannel;
      // get the list of channels and modes for that user
      globalIndex = list_locate(updatingUser->channelModes, channel); 
      userChannel = (forChannel *) list_get_at(updatingUser->channelModes, globalIndex);
      pthread_mutex_unlock(&channel->chanUserLock);
      if (adjMode[0] == '+')
      {
        if (adjMode[1] != 'v' && adjMode[1] != 'o')
        {
          // return an error
          // incorrect flags
          memcpy(reply->responseCode, ERR_UNKNOWNMODE, REPLYCODELEN);
          reply->numArgs = 2;
          argLen = 1 + strlen(channel->name) + reply->numArgs;
          snprintf(reply->args, argLen, "%c %s", adjMode[1], channel->name);
          reply->args[argLen] = '\0';
          send_response(info->socket, reply);
          return;
        }
        // if the first spot has the flag
        userChannel->modes[0] = adjMode[1];
        // send confirmation
      }
      else if (adjMode[0] == '-')
      {
        if (adjMode[1] != 'v' && adjMode[1] != 'o')
        {
          // return an error
          // incorrect flags
          memcpy(reply->responseCode, ERR_UNKNOWNMODE, REPLYCODELEN);
          reply->numArgs = 2;
          argLen = 1 + strlen(channel->name) + reply->numArgs;
          snprintf(reply->args, argLen, "%c %s", adjMode[1], channel->name);
          reply->args[argLen] = '\0';
          send_response(info->socket, reply);
          return;
        }
        // if the first spot has the flag
        if (userChannel->modes[0] == adjMode[1])
        {
          // set flag to NULL
          memset(&userChannel->modes[0], 0, 1);
          // send confirmation
        }
      }
      int replyBeginLen = 1 + strlen(info->nickname) + // account for colon
                          1 + strlen(info->username) + // account for bang
                          1 + strlen(info->host) + // account for @
                          2; // account for spaces
      char replyBeginning[replyBeginLen];
      snprintf(replyBeginning, replyBeginLen, ":%s!%s@%s ", info->nickname,
                                                            info->username,
                                                            info->host);
      int replyEndLen = strlen("MODE") +
                        strlen(channel->name) +
                        strlen(adjMode) +
                        strlen(secondName) + 5; // account for # and spaces
      char replyEnd[replyEndLen];
      snprintf(replyEnd, replyEndLen, "MODE #%s %s %s", channel->name, adjMode, secondName);
      userInfo * recieving_user;
      pthread_mutex_lock(&channel->chanUserLock);
      list_iterator_start(channel->userList);
      while (list_iterator_hasnext(channel->userList))
      {
        recieving_user = (userInfo *) list_iterator_next(channel->userList);
        send(recieving_user->socket, replyBeginning, replyBeginLen, 0);
        send(recieving_user->socket, replyEnd, replyEndLen, 0);
        send(recieving_user->socket, "\r\n", 2, 0);
      }
      list_iterator_stop(channel->userList);
      pthread_mutex_unlock(&channel->chanUserLock);
      // globally update user list
      pthread_mutex_lock(&lock);
      int userIndex = list_locate(userList, updatingUser);
      list_delete_at(userList, userIndex);
      list_insert_at(userList, updatingUser, userIndex);
      list_sort(userList, -1);
      pthread_mutex_unlock(&lock);
      return;
      // end subtraction case
    }
    // end user channel specific if
  }
  // end first argument was channel case
  else
  {
    userInfo * user;
    if (strcmp(firstName, info->nickname))
    {
      // return unmatched users error
      reply->numArgs = 0;
      memcpy(reply->responseCode, ERR_USERSDONTMATCH, REPLYCODELEN);
      send_response(info->socket, reply);
      return;
    }
    pthread_mutex_lock(&lock);
    if (!(user = (userInfo *) list_seek(userList, &firstName)))
    {
      pthread_mutex_unlock(&lock);
      return;
      // return an error, there's no such user
    }
    pthread_mutex_unlock(&lock);
    if (adjMode[0] == '+')
    {
      if (adjMode[1] != 'o' && adjMode[1] != 'a')
      {
        reply->numArgs = 0;
        memcpy(reply->responseCode, ERR_UMODEUNKNOWNFLAG, REPLYCODELEN);
        send_response(info->socket, reply);
        return;
        // return an error
        // incorrect flags
      }
    }
    else if (adjMode[0] == '-')
    {
      if (adjMode[1] != 'o' && adjMode[1] != 'a')
      {
        // return an error
        // incorrect flags
        reply->numArgs = 0;
        memcpy(reply->responseCode, ERR_UMODEUNKNOWNFLAG, REPLYCODELEN);
        send_response(info->socket, reply);
        return;
      }
      // if the first spot has the flag
      if (adjMode[1] == 'o')
      {
        if (user->modes[0] == adjMode[1])
        {
          // move the second flag to the first position, set the second flag to NULL
          user->modes[0] = user->modes[1];
          memset(&user->modes[1], 0, 1);
          // send confirmation
        }
        else if (user->modes[1] == adjMode[1])
        {
          // if the second flag matches, set that flag to NULL
          memset(&user->modes[1], 0, 1);
          // send confirmation
        }
        int replyBeginLen = strlen(info->nickname) +
                            strlen("MODE") +
                            strlen(info->nickname) +
                            strlen(adjMode) + 6; // account for colon and spaces
        char replyBeginning[replyBeginLen];
        snprintf(replyBeginning, replyBeginLen, ":%s MODE %s :%s", info->nickname, info->nickname, adjMode);
        pthread_mutex_lock(&lock);
        send(info->socket, replyBeginning, replyBeginLen, 0);
        send(info->socket, "\r\n", 2, 0);
        pthread_mutex_unlock(&lock);
      }
    }
    // end subtraction case
  }
  // end user case
}


void oper(char * password, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  if (strcmp(password, servData->passwd))
  {
    memcpy(reply->responseCode, ERR_PASSWDMISMATCH, REPLYCODELEN);
    reply->numArgs = 0;
    send_response(info->socket, reply);
    return;
  }
  pthread_mutex_lock(&lock);
  if (info->modes[0] == 'a')
  {
    info->modes[1] = 'o';
    info->modes[2] = '\0';
  }
  else
  {
    info->modes[0] = 'o';
    info->modes[1] = '\0';
  }
  int globalIndex = list_locate(userList, info);
  list_delete_at(userList, globalIndex);
  list_insert_at(userList, info, globalIndex);
  list_sort(userList, -1);
  list_iterator_start(info->channelModes);
  while (list_iterator_hasnext(info->channelModes))
  {
    forChannel * channelMode = (forChannel *) list_iterator_next(info->channelModes);
    char * chanName = channelMode->channelName;
    pthread_mutex_lock(&chanLock);
    channelData * channel = (channelData *) list_seek(chanList, &chanName);
    pthread_mutex_unlock(&chanLock);
    pthread_mutex_lock(&channel->chanUserLock);
    int userIndex = list_locate(channel->userList, info);
    list_delete_at(channel->userList, userIndex);
    list_insert_at(channel->userList, info, userIndex);
    list_sort(channel->userList, 1);
    pthread_mutex_unlock(&channel->chanUserLock);
  }
  list_iterator_stop(info->channelModes);
  pthread_mutex_unlock(&lock);
  memcpy(reply->responseCode, RPL_YOUREOPER, REPLYCODELEN);
  reply->numArgs = 0;
  send_response(info->socket, reply);
  return;
}


void away(char * msg, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int globalIndex;
  if (msg == NULL)
  {
    if (info->modes[0] == 'a')
    {
      if (info->modes[1] == 'o')
        memcpy(&info->modes[0], &info->modes[1], 2);
      else
        memset(&info->modes[0], 0, 1);
    }
    else if (info->modes[1] == 'a')
    {
      info->modes[1] = '\0';
    }
    pthread_mutex_lock(&lock);
    memset(info->away, 0, MAXAWAY);
    globalIndex = list_locate(userList, info);
    list_delete_at(userList, globalIndex);
    list_insert_at(userList, info, globalIndex);
    list_sort(userList, -1);
    pthread_mutex_unlock(&lock);
    memcpy(reply->responseCode, RPL_UNAWAY, REPLYCODELEN);
    reply->numArgs = 0;
    send_response(info->socket, reply);
    return;
  }
  pthread_mutex_lock(&lock);
  if (info->modes[0] == 'o')
  {
    info->modes[1] = 'a';
    info->modes[2] = '\0';
  }
  else
  {
    info->modes[0] = 'a';
    info->modes[1] = '\0';
  }  
  memcpy(info->away, msg, strlen(msg));
  info->away[strlen(msg)-1] = '\0';
  globalIndex = list_locate(userList, info);
  list_delete_at(userList, globalIndex);
  list_insert_at(userList, info, globalIndex);
  list_sort(userList, -1);
  list_iterator_start(info->channelModes);
  while (list_iterator_hasnext(info->channelModes))
  {
    forChannel * channelMode = (forChannel *) list_iterator_next(info->channelModes);
    char * chanName = channelMode->channelName;
    pthread_mutex_lock(&chanLock);
    channelData * channel = (channelData *) list_seek(chanList, &chanName);
    pthread_mutex_unlock(&chanLock);
    pthread_mutex_lock(&channel->chanUserLock);
    int userIndex = list_locate(channel->userList, info);
    list_delete_at(channel->userList, userIndex);
    list_insert_at(channel->userList, info, userIndex);
    list_sort(channel->userList, 1);
    pthread_mutex_unlock(&channel->chanUserLock);
  }
  list_iterator_stop(info->channelModes);
  pthread_mutex_unlock(&lock);
  memcpy(reply->responseCode, RPL_NOWAWAY, REPLYCODELEN);
  reply->numArgs = 0;
  send_response(info->socket, reply);
}

void names(char * chanName, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  int argLen;
  char userChanMode = '=';

  // if no channel name provided, give info on all channels
  if (chanName == NULL)
  {
    pthread_mutex_lock(&chanLock);
    list_iterator_start(chanList);
    while (list_iterator_hasnext(chanList))
    {
      channelData * channel = (channelData *) list_iterator_next(chanList);
      pthread_mutex_unlock(&chanLock);
      memcpy(reply->responseCode, RPL_NAMREPLY, REPLYCODELEN);
      // there should be a max num of users in a channel FIX
      reply->numArgs = 2;
      argLen = strlen(channel->name) + reply->numArgs + 2; // account for # and user chan mode
      snprintf(reply->args, argLen, "%c #%s", userChanMode, channel->name);
      reply->args[argLen] = '\0';

      memset(reply->message, 0, 512);
      int totalReplyLen = 0;
      pthread_mutex_lock(&channel->chanUserLock);
      list_iterator_start(channel->userList);
      while (list_iterator_hasnext(channel->userList))
      {
        userInfo * user = (userInfo *) list_iterator_next(channel->userList);
        char * channelName = (char *) malloc(strlen(channel->name)*sizeof(char));
        memcpy(channelName, channel->name, strlen(channel->name));
        forChannel * userChannel = (forChannel *) list_seek(user->channelModes, &channelName); 
        if (userChannel)
        {
          if (userChannel->modes[0] == 'o')
          {
            memcpy(reply->message+totalReplyLen, "@", 1);
            totalReplyLen++;
          }
          else if (userChannel->modes[0] == 'v')
          {
            memcpy(reply->message+totalReplyLen, "+", 1);
            totalReplyLen++;
          }
        }
        memcpy(reply->message+totalReplyLen, user->nickname, strlen(user->nickname));
        totalReplyLen += strlen(user->nickname) + 1; // account for space char
        reply->message[totalReplyLen-1] = ' ';
      }
      list_iterator_stop(channel->userList);
      pthread_mutex_unlock(&channel->chanUserLock);
      if (totalReplyLen > 0)
        reply->message[totalReplyLen-1] = '\0';
      send_response(info->socket, reply);
      pthread_mutex_lock(&chanLock);
    }
    list_iterator_stop(chanList);
    pthread_mutex_unlock(&chanLock);
   
    // list all users not in channels
    memcpy(reply->responseCode, RPL_NAMREPLY, REPLYCODELEN);
    // there should be a max num of users in a channel FIX
    reply->numArgs = 2;
    argLen = strlen("*") + reply->numArgs + 1; // account for user chan mode
    userChanMode = '*';
    snprintf(reply->args, argLen, "%c %s", userChanMode, "*");
    memset(reply->message, 0, 512);
    int totalReplyLen = 0;
    pthread_mutex_lock(&lock);
    list_iterator_start(userList);
    while(list_iterator_hasnext(userList))
    {
      userInfo * user = (userInfo *) list_iterator_next(userList);
      if (list_size(user->channelModes) == 0)
      {
        memcpy(reply->message+totalReplyLen, user->nickname, strlen(user->nickname));
        totalReplyLen += strlen(user->nickname) + 1; // account for space char
        reply->message[totalReplyLen-1] = ' ';
      }
    }
    list_iterator_stop(userList);
    pthread_mutex_unlock(&lock);
    if (totalReplyLen > 0)
    {
      reply->message[totalReplyLen-1] = '\0';
      send_response(info->socket, reply);
    }
  }   // if channel name provided, only give info on channel
  else
  {
    // remove # from chanName
    if (chanName[0] == '#')
    {
      memcpy(chanName, &chanName[1], strlen(chanName)-1);
      chanName[strlen(chanName)-1] = '\0';
    }
    // figure out when to return ERR_NOSUCHCHANNEL
    channelData * channel;
    // if chanName is not part of chanList, create new channelData structure
    pthread_mutex_lock(&chanLock);
    channel = list_seek(chanList, &chanName);
    pthread_mutex_unlock(&chanLock);

    if (channel != NULL)
    {
      memcpy(reply->responseCode, RPL_NAMREPLY, REPLYCODELEN);
      // there should be a max num of users in a channel FIX
      reply->numArgs = 2;
      argLen = strlen(channel->name) + reply->numArgs + 2; // account for # and user channel mode
      snprintf(reply->args, argLen, "%c #%s", userChanMode, channel->name);
      reply->args[argLen] = '\0';

      memset(reply->message, 0, 512);
      int totalReplyLen = 0;
      pthread_mutex_lock(&channel->chanUserLock);
      list_iterator_start(channel->userList);
      while (list_iterator_hasnext(channel->userList))
      {
        userInfo * user = (userInfo *) list_iterator_next(channel->userList);
        forChannel * userChannel = (forChannel *) list_seek(user->channelModes, &chanName);
        if (userChannel->modes[0] == 'o')
        {
          memcpy(reply->message+totalReplyLen, "@", 1);
          totalReplyLen++;
        }
        else if (userChannel->modes[0] == 'v')
        {
          memcpy(reply->message+totalReplyLen, "+", 1);
          totalReplyLen++;
        }
        memcpy(reply->message+totalReplyLen, user->nickname, strlen(user->nickname));
        totalReplyLen = totalReplyLen + strlen(user->nickname) + 1; // account for space char
        reply->message[totalReplyLen-1] = ' ';
      }
      list_iterator_stop(channel->userList);
      reply->message[totalReplyLen-1] = '\0';
      pthread_mutex_unlock(&channel->chanUserLock);
      send_response(info->socket, reply);
    }
  }
  // send RPL_ENDOFNAMES
  memcpy(reply->responseCode, RPL_ENDOFNAMES, REPLYCODELEN);
  reply->numArgs = 1;
  if (chanName == NULL)
  {
    argLen = 1 + reply->numArgs; // account for *
    snprintf(reply->args, argLen, "%s", "*");
  }
  else
  {
    argLen = strlen(chanName) + reply->numArgs + 1; // account for #
    snprintf(reply->args, argLen, "#%s", chanName);
  }
  send_response(info->socket, reply);
  return;
}


void who(char * mask, userInfo * info, list_t * userList, list_t * chanList, replyPackage * reply, serverInfo * servData)
{
  // if there's no mask
  if (strlen(mask) == 1 && (mask[0] == '*' || mask[0] == '0'))
  {
    pthread_mutex_lock(&lock);
    list_t * send_to;
    send_to = (list_t *) malloc(sizeof(list_t));
    list_init(send_to);
    list_attributes_copy(send_to, user_info_size, 1);
    list_attributes_comparator(send_to, nick_comparator);
    list_attributes_seeker(send_to, (element_seeker) seeker);
    pthread_mutex_unlock(&lock);
    channelData * channel;
    userInfo *about_user = malloc(sizeof(userInfo));
    char *from_user = info->nickname;
    pthread_mutex_lock(&chanLock);
    list_iterator_start(chanList);
    //for each channel
    while (list_iterator_hasnext(chanList))
    {
      channel = (channelData *) list_iterator_next(chanList);
      pthread_mutex_unlock(&chanLock);
      //if user is on channel
      pthread_mutex_lock(&channel->chanUserLock);
      if((list_seek(channel->userList, &from_user)))
      {
        list_iterator_start(channel->userList);
        //look at the user list
        while (list_iterator_hasnext(channel->userList))
        {
          about_user = (userInfo *) list_iterator_next(channel->userList);
          pthread_mutex_unlock(&channel->chanUserLock);
          list_append(send_to, about_user);
          list_sort(send_to, -1);
          pthread_mutex_lock(&channel->chanUserLock);
        }
        list_iterator_stop(channel->userList);
        pthread_mutex_unlock(&channel->chanUserLock);
      }
      else
        pthread_mutex_unlock(&channel->chanUserLock);
      pthread_mutex_lock(&chanLock);
    }
    list_iterator_stop(chanList);
    pthread_mutex_unlock(&chanLock);

    pthread_mutex_lock(&lock);
    list_iterator_start(userList);
    char * tempname;
    while (list_iterator_hasnext(userList))
    {
      about_user = (userInfo *) list_iterator_next(userList);
      tempname = about_user->nickname;
      if (list_seek(send_to, &tempname))
        continue;

      char ircOp = 'n';
      char status = 'n';
      int replyEndLen = strlen(mask) +
                        strlen(about_user->username) +
                        strlen(about_user->host) +
                        strlen(servData->serverHost) +
                        strlen(about_user->nickname) +
                        strlen(about_user->name) + 12; // account for spaces, :0, status, and voic_oper

      char replyEnd[replyEndLen];
      if (about_user->modes[0] == 'a' || about_user->modes[1] == 'a')
        status = 'G';
      else
        status = 'H';
      if (about_user->modes[0] == 'o' || about_user->modes[1] == 'o')
        ircOp = '*';
      int totFlags = 4;
      char * flags = (char *) malloc(sizeof(char)*totFlags);
      memset(flags, 0, totFlags);
      flags[0] = status;
      if (ircOp != 'n')
        flags[1] = ircOp;
      snprintf(replyEnd, replyEndLen, "%s %s %s %s %s %s :0 %s", mask,
                                                                 about_user->username,
                                                                 about_user->host,
                                                                 servData->serverHost,
                                                                 about_user->nickname,
                                                                 flags,
                                                                 about_user->name);


      memcpy(reply->responseCode, RPL_WHOREPLY, REPLYCODELEN);
      reply->numArgs = 0;

      memset(reply->message, 0, 512);
      memcpy(reply->message, replyEnd, strlen(replyEnd));
      pthread_mutex_unlock(&lock);
      send_response(info->socket, reply);
      pthread_mutex_lock(&lock);
    }
    list_iterator_stop(userList);
    pthread_mutex_unlock(&lock);
    memcpy(reply->responseCode, RPL_ENDOFWHO, REPLYCODELEN);
    reply->numArgs = 1;
    int argLen = strlen(mask)+1;
    snprintf(reply->args, argLen, "%s", mask);
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
  }

  // if there's a mask
  else
  {
    if (mask[0] == '#')
    {
      memcpy(mask, &mask[1], strlen(mask)-1);
      mask[strlen(mask)-1] = '\0';
    }
    channelData * channel;
    pthread_mutex_lock(&chanLock);  
    if (!(channel = list_seek(chanList, &mask)))
    {
      pthread_mutex_unlock(&chanLock);
      // return error, no such channel
      return;
    }
    else
      pthread_mutex_unlock(&chanLock);
    forChannel * forChan;
    pthread_mutex_lock(&channel->chanUserLock);
    list_iterator_start(channel->userList);
    while (list_iterator_hasnext(channel->userList))
    {
      char ircOp = 'n';
      char status = 'n';
      char voic_oper = 'n';
      userInfo * about_user = (userInfo *) list_iterator_next(channel->userList);
      char * tempChanName = channel->name;
      forChan = list_seek(about_user->channelModes, &tempChanName);
      int replyEndLen = strlen(channel->name) +
                        strlen(about_user->username) +
                        strlen(about_user->host) +
                        strlen(servData->serverHost) +
                        strlen(about_user->nickname) + 
                        strlen(about_user->name) + 13; // account for #, :, spaces, status, and voic_op
      char replyEnd[replyEndLen];

      if (about_user->modes[0] == 'a' || about_user->modes[1] == 'a')
        status = 'G';
      else
        status = 'H';
      if (about_user->modes[0] == 'o' || about_user->modes[1] == 'o')
      {
        ircOp = '*';
      }
      if (forChan->modes[0] == 'o' || forChan->modes[1] == 'o')
        voic_oper = '@';
      else if (forChan->modes[0] == 'v' || forChan->modes[1] == 'v')
        voic_oper = '+';
      int totFlags = 4;
      char * flags = (char *) malloc(sizeof(char)*totFlags);
      memset(flags, 0, totFlags);
      flags[0] = status;
      if (ircOp != 'n')
      {
        flags[1] = ircOp;
        if (voic_oper != 'n')
          flags[2] = voic_oper;
      }
      else if (voic_oper != 'n')
        flags[1] = voic_oper;

      snprintf(replyEnd, replyEndLen, "#%s %s %s %s %s %s :0 %s", channel->name,
                                                                  about_user->username,
                                                                  about_user->host,
                                                                  servData->serverHost,
                                                                  about_user->nickname,
                                                                  flags,
                                                                  about_user->name);
      memcpy(reply->responseCode, RPL_WHOREPLY, REPLYCODELEN);
      reply->numArgs = 0;
      memset(reply->message, 0, 512);
      memcpy(reply->message, replyEnd, strlen(replyEnd));
      send_response(info->socket, reply);
    }
    list_iterator_stop(channel->userList);
    pthread_mutex_unlock(&channel->chanUserLock);
    
    memcpy(reply->responseCode, RPL_ENDOFWHO, REPLYCODELEN);
    reply->numArgs = 1;
    int argLen;
    if (channel)
    {
      argLen = strlen(mask) + 1 + reply->numArgs; // account for #
      snprintf(reply->args, argLen, "#%s", mask);
    }
    else
    {
      argLen = strlen(mask) + reply->numArgs;
      snprintf(reply->args, argLen, "%s", mask);
    }
    reply->args[argLen] = '\0';
    send_response(info->socket, reply);
  }
}
    
