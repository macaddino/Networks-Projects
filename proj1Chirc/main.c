/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *  Isabelle Rice and Laura Macaddino
 *  main() code for chirc project
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include "command.h"
#include "globalData.h"
#include "listfxns.h"
#include "parser.h"
#include "reply.h"
#include "structures.h" 

extern pthread_mutex_t lock;
extern pthread_mutex_t chanLock;
extern int num_pthreads;

/* run_client:
 * This is the function which each spawned p_thread will
 * run. Parses commands from clients and responds to 
 * client accordingly.
 */
void *run_client(void *args){
    struct workerArgs *wa;
    wa = (struct workerArgs*) args;

    pthread_detach(pthread_self());

    pthread_mutex_lock(&lock);
    num_pthreads++;
    pthread_mutex_unlock(&lock);

    // setting total amount of chars buf can parse
    // at a time. (Command max len is handled in break_commands)
    int bufLen = 6000;
    // setting total amount of commands which can be parsed at
    // at time.
    int commandBufLen = 250;
    int nbytes;
    int n;
    char inputBuf[bufLen];
    char buildBuf[bufLen];
    char **commandList;
    int buildLen = 0;
    memset(inputBuf, 0, bufLen);
    memset(buildBuf, 0, bufLen);
    commandList = (char **) malloc(COMMANDNUM*sizeof(char **));
    command_init(commandList);
    userInfo * info;
    info = (userInfo *) malloc(sizeof(userInfo));
    memset(info, 0, sizeof(userInfo));
    int clientSocket = wa->socket;
    char * clientHost = wa->clientHost;

    int hostLen = strlen(clientHost);
    if (hostLen > MAXHOST)
      hostLen = MAXHOST;
    memcpy(info->host, clientHost, hostLen);
    info->host[hostLen] = '\0';
    info->socket = clientSocket;

    list_t * userList = wa->userList;
    list_t * chanList = wa->chanList;
    struct serverInfo * servData = wa->servData;

    // collect input from client until disconnect
    // build input buffer until buffer terminates with "\r\n"
    while( (nbytes = recv(clientSocket, inputBuf, bufLen, 0)) )
    {
      if (nbytes + buildLen > bufLen)
        nbytes = bufLen - buildLen;
      memcpy(buildBuf+buildLen, inputBuf, nbytes);
      buildLen = buildLen + nbytes;
      // handles case if string is too long for buffer
      if ((buildBuf[bufLen-1]) && (buildBuf[bufLen-1]!='\n'))
      {
        buildBuf[bufLen-2]='\r';
        buildBuf[bufLen-1]='\n';
      }
      // if input buffer ends with \r\n, parse buffer and run commands
      if (buildBuf[buildLen-1]=='\n' && buildBuf[buildLen-2]=='\r')
      {
        int maxArgs = 15;
        char ** argList;

        argList = (char **) malloc(maxArgs*sizeof(char *));
	      memset(argList, 0, sizeof(argList));
        char ** cmndList;
        cmndList = (char **) malloc(commandBufLen*sizeof(char *));
	      memset(cmndList, 0, sizeof(cmndList));

        // determine how many commands are stored in buffer
        int numCmnds = break_commands(buildBuf, buildLen, cmndList);
        int command;

        for (n=0; n<numCmnds; n++)
        {
          int argNum = parser(cmndList[n], strlen(cmndList[n]), argList);
          command = command_search(argList[0], commandList);
          if (command == -1)
          { 
            replyPackage reply;
            memset(&reply, 0, sizeof(replyPackage));
            memcpy(reply.serverName, servData->serverHost, strlen(servData->serverHost));
            if (info->nickname[0])
              memcpy(reply.nickname, info->nickname, strlen(info->nickname));
            else
              memcpy(reply.nickname, "*", 1);
            memcpy(reply.responseCode, ERR_UNKNOWNCOMMAND, REPLYCODELEN);
            reply.numArgs = 1;
            int argLen = strlen(argList[0]) + reply.numArgs;
            snprintf(reply.args, argLen, "%s", argList[0]);
            reply.args[argLen] = '\0';
            send_response(clientSocket, &reply);
  
          }
          else
            run_command(command, argList, argNum, info, userList, chanList, servData);
        }
        free(argList);
        free(cmndList);
        buildLen = 0;
        memset(inputBuf, 0, bufLen);
        memset(buildBuf, 0, bufLen);
      }
    }
    free(wa);

    pthread_mutex_lock(&lock);
    num_pthreads--;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
  time_t current_time;
  char * createdDate;
  int opt;
  char *port = "6667", *passwd = NULL;


  while ((opt = getopt(argc, argv, "p:o:h")) != -1)
    switch (opt)
    {
      case 'p':
        port = strdup(optarg);
        break;
      case 'o':
        passwd = strdup(optarg);
        break;
      default:
        printf("ERROR: Unknown option -%c\n", opt);
        exit(-1);
    }

  if (!passwd)
  {
    fprintf(stderr, "ERROR: You must specify an operator password\n");
    exit(-1);
  }
  num_pthreads = 0;
  int serverSocket;
  int clientSocket;
  pthread_t worker_thread;
  struct sockaddr_in serverAddr, clientAddr;
  int yes = 1; 
  socklen_t sinSize = sizeof(struct sockaddr_in);

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(atoi(port));
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  // store server data
  serverInfo * servData;
  servData = (serverInfo *) malloc(sizeof(serverInfo));
  memset(servData, 0, sizeof(servData));
  struct hostent *heServ;
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  heServ = gethostbyname(hostname);
  memcpy(servData->passwd, passwd, strlen(passwd));
  memcpy(servData->serverHost, heServ->h_name, strlen(heServ->h_name));
  char serverVersion[] = "version2";
  memcpy(servData->serverVersion, serverVersion, strlen(serverVersion));
  current_time = time(NULL);
  createdDate = ctime(&current_time);
  memcpy(servData->createdDate, createdDate, strlen(createdDate));
  char userModes[] = "ao";
  memcpy(servData->userModes, userModes, strlen(userModes));
  char chanModes[] = "mtov";
  memcpy(servData->chanModes, chanModes, strlen(chanModes));

  serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  listen(serverSocket, 5);

  // initialize global list of users
  list_t * userList = (list_t *) malloc(sizeof(list_t));
  list_init(userList);
  list_attributes_copy(userList, user_info_size, 1);
  list_attributes_comparator(userList, nick_comparator);
  list_attributes_seeker(userList, (element_seeker) seeker);

  // initialize global list of channels
  list_t * chanList = (list_t *) malloc(sizeof(list_t));
  list_init(chanList);
  list_attributes_copy(chanList, chan_info_size, 1);
  list_attributes_comparator(chanList, chan_comparator);
  list_attributes_seeker(chanList, (element_seeker) chan_seeker);

  // initialize list of users not in a channel
/*  channelData * newChannel;
  newChannel = (channelData *) malloc(sizeof(channelData));
  memcpy(newChannel->name, "*\0", strlen("*\0"));
  pthread_mutex_init(&newChannel->chanUserLock, NULL);
  // remember to delete mutex upon channel deletion
  newChannel->userList = (list_t *) malloc(sizeof(list_t));
  list_init(newChannel->userList);
  list_attributes_copy(newChannel->userList, user_info_size, 1);
  list_attributes_comparator(newChannel->userList, nick_comparator);
  list_attributes_seeker(newChannel->userList, (element_seeker) seeker);
  list_append(chanList, newChannel);
  list_sort(chanList, 1);
*/
  struct workerArgs *wa;

  pthread_mutex_init(&lock, NULL);
  pthread_mutex_init(&chanLock, NULL);

  while(1)
  {
    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
    setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    // retrieve client hostname
    struct hostent *he;
    struct in_addr ipv4addr;
    char *IP = inet_ntoa(clientAddr.sin_addr);
    inet_pton(AF_INET, IP, &ipv4addr);
    he = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    char *clientHost = he->h_name;
    // pass on arguments to worker thread
    wa = malloc(sizeof(struct workerArgs));
    wa->socket = clientSocket;
    wa->clientHost = clientHost;
    wa->userList = userList;
    wa->chanList = chanList;
    wa->servData = servData;
    if(pthread_create(&worker_thread, NULL, run_client, wa) != 0)
    {
      perror("Could not create a worker thread");
      free(wa);
      close(clientSocket);
      pthread_exit(NULL);
    }
  }
  close(serverSocket);
  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&chanLock);
  return 0;
}


