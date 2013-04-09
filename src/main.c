/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *  Isabelle Rice and Laura Macaddino
 *  main() code for chirc project
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
#include "listfxns.h"
#include "parser.h"
#include "structures.h" 


int main(int argc, char *argv[])
{
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

        int serverSocket;
	int clientSocket;
	struct sockaddr_in serverAddr, clientAddr;
	int yes = 1; 
	socklen_t sinSize = sizeof(struct sockaddr_in);

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(port));
        serverAddr.sin_addr.s_addr = INADDR_ANY;

	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	listen(serverSocket, 5);

	list_t userList;
	list_init(&userList);
	list_attributes_copy(&userList, user_info_size, 1);

	while(1)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
		// retrieve client hostname
		struct hostent *he;
		struct in_addr ipv4addr;
		char *IP = inet_ntoa(clientAddr.sin_addr);
		inet_pton(AF_INET, IP, &ipv4addr);
		he = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
		char *clientHost = he->h_name;

		int msgLen = 512;
                int nbytes;
		int n;
		char inputBuf[msgLen];
		char buildBuf[msgLen];
                char **commandList;
		int buildLen = 0;
		memset(inputBuf, 0, msgLen);
		memset(buildBuf, 0, msgLen);
		commandList = (char **) malloc(COMMANDNUM*sizeof(char **));
		command_init(commandList);
		userInfo info = {{ 0 }};

		// collect input from client until disconnect
		// build input buffer until buffer terminates with "\r\n"
		while( (nbytes = recv(clientSocket, inputBuf, msgLen, 0)) )
		{
			memcpy(buildBuf+buildLen, inputBuf, nbytes); 
			buildLen = buildLen + nbytes;

			// handles case if string is too long
			if ((buildBuf[msgLen-1]) && (buildBuf[msgLen-1]!='\n'))
			{
				buildBuf[msgLen-2]='\r';
				buildBuf[msgLen-1]='\n';
			}
			// if input buffer ends with \r\n, parse buffer and run commands
			if (buildBuf[buildLen-1]=='\n' && buildBuf[buildLen-2]=='\r')
			{
				char **argList;
				int maxArgs = 15;
				argList = (char **) malloc(maxArgs*sizeof(char **));
				char **cmndList;
				cmndList = (char **) malloc(msgLen*sizeof(char **));
				// determine how many commands are stored in buffer
				int numCmnds = break_commands(buildBuf, buildLen, cmndList);
				for (n=0; n<numCmnds; n++)
				{
					int argNum = parser(cmndList[n], strlen(cmndList[n]), argList);
					int command = command_search(argList[0], commandList);
					run_command(command, argList, argNum, &info, &userList, clientSocket, clientHost);
				}
				free(argList);
				buildLen = 0;
				memset(inputBuf, 0, msgLen); 
				memset(buildBuf, 0, msgLen);
			}
		}
	}
	close(serverSocket);
	return 0;
}

