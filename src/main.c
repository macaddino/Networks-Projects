/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
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

/* parser: 
 * Given an array containing a command, its number of bytes,
 * and an empty 2D char array, fills the 2D char array with
 * each individual word in the command and returns the 
 * number of arguments in the command (total number of words
 * in the command minus one).
 */
int parser(char *input,  int nbytes, char **argList)
{
	//terminates with "\r\n"

	int maxArgs = 15;	
	int array_index = 0;
	int start_cpy = 0;
	int n;
	for (n=0; n<nbytes-1; n++)
	{
		if (input[n] == ' ')
		{
			int argSize = n-1-start_cpy;
			argList[array_index] = (char *) malloc(argSize);
			memcpy(argList[array_index], input+start_cpy, argSize);
			start_cpy = n+1;
			array_index++;
		}
		if (array_index == maxArgs)
		{ //prevent more than 15 arguments from being stored
			break;
		}
	}
	return array_index;
}

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
	int yes = 1; // I'm not sure what this variable is
	socklen_t sinSize = sizeof(struct sockaddr_in);

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(port));
        serverAddr.sin_addr.s_addr = INADDR_ANY;

	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
	listen(serverSocket, 5);

	while(1)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
		int msgLen = 512;
		char inputBuf[msgLen];
		memset(inputBuf, 0, msgLen);

		int nbytes;
		while( (nbytes = recv(clientSocket, inputBuf, msgLen, 0)) )
		{
			//handles case if string is too long
			if ((inputBuf[msgLen-1]) && (inputBuf[msgLen-1]!='\n'))
			{
				inputBuf[msgLen-2]='\r';
				inputBuf[msgLen-1]='\n';//should a message be saved if it is cut off?
			}
			printf("I got your message! It is: %s", inputBuf); //remove later
			char **argList;
			int maxArgs = 15;
			argList = (char **) malloc((1+maxArgs)*sizeof(char **));
			int argNum = parser(inputBuf, nbytes, argList);
			printf("Number of args:%d\n", argNum); //remove later 

			free(argList);

			memset(inputBuf, 0, msgLen); //clear buffer
		}
	}

	close(serverSocket);


	return 0;
}

