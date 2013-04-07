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
#include "reply.h"
#include "command.h"
#include "simclist.h"

struct userInfo
{
	char nickname[9];
	char username[9];
	char name[50];
};

typedef struct userInfo userInfo;

/* parser: 
 * Given an array containing a command, its number of bytes,
 * and an empty 2D char array, fills the 2D char array with
 * each individual word in the command and returns the 
 * number of words in the command.
 */
int parser(char *input,  int nbytes, char **argList)
{

	int maxArgs = 15;	
	int array_index = 0;
	int start_cpy = 0;
	int n;
	for (n=0; n<nbytes; n++)
	{
		if ((input[n] == ' ') || (n == nbytes-2)) //cuts off \r\n
		{
			int argSize = n-start_cpy;
			argList[array_index] = (char *) malloc(argSize+1);
			memcpy(argList[array_index], input+start_cpy, argSize);
			argList[array_index][argSize] = '\0'; // Terminate string with NULL
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

/* command_init:
 * Initializes a list
 * of possible chIRC commands.
 */
void command_init(char ** commands)
{
	commands[0] = "NICK";
	commands[1] = "USER";
}

size_t user_info_size(const void *el)
{
	return sizeof(userInfo);
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


int send_response(int clientSocket, char * response, userInfo * info)
{
        char serverName[] = ":irc.example.com";
        char clientHost[] = "fool.cs.uchicago.edu";

	if (!(strcmp(response, RPL_WELCOME)))
	{	
                send(clientSocket, serverName, strlen(serverName), 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, RPL_WELCOME, 3, 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, info->nickname, strlen(info->nickname), 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, RPL_WELCOME_MSG, strlen(RPL_WELCOME_MSG), 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, info->nickname, strlen(info->nickname), 0);
                send(clientSocket, "!", 1, 0);
                send(clientSocket, info->username, strlen(info->username), 0);
                send(clientSocket, "@", 1, 0);
                send(clientSocket, clientHost, strlen(clientHost), 0);
		send(clientSocket, "\r\n", 2, 0);
	}
	else if (!(strcmp(response, ERR_UNKNOWNCOMMAND)))
	{
		send(clientSocket, serverName, strlen(serverName), 0);
                send(clientSocket, " ", 1, 0);
		send(clientSocket, ERR_UNKNOWNCOMMAND, 3, 0);
		send(clientSocket, " ", 1, 0);
		if (info->nickname[0])
			send(clientSocket, info->nickname, strlen(info->nickname), 0);
		else
			send(clientSocket, "*", 1, 0);
		send(clientSocket, " ", 1, 0);
                send(clientSocket, ERR_UNKNOWNCOMMAND_MSG, strlen(ERR_UNKNOWNCOMMAND_MSG), 0);		
		send(clientSocket, "\r\n", 2, 0);
	}
	else if (!(strcmp(response, ERR_ALREADYREGISTRED)))
	{
		send(clientSocket, serverName, strlen(serverName), 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, ERR_ALREADYREGISTRED, 3, 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, info->nickname, strlen(info->nickname), 0);
                send(clientSocket, " ", 1, 0);
                send(clientSocket, ERR_ALREADYREGISTERED_MSG, strlen(ERR_ALREADYREGISTERED_MSG), 0);
                send(clientSocket, "\r\n", 2, 0);

	}
	else
	{
                printf("Invalid response.\n");
                return -1;
        }
	
        return 0;
}


void nick(char * nickname, userInfo * info, list_t * userList, int cliSocket) //perhaps this should take a userInfo struct.
{
	int maxNickLen = 9;
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
		// now, sort the list based on nicki
		send_response(cliSocket, RPL_WELCOME, info);
		printf("Welcome to the server!\n");
		// send welcome message back from server
	}
	else if (!(isFirstNick))
	{
		printf("Check to see if list has changed!\n");
		// update list CHECK TO SEE IF ALREADY UPDATED!!
	}

	printf("User struct says NICK is %s\n", info->nickname);

}

void user(char * username, char * name, userInfo * info, list_t * userList, int cliSocket)
{
	int maxUserLen = 9;
	int maxNameLen = 50;
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

	printf("This is initial username: %s\n", info->username);
	if ((info->username[0]) && (info->nickname[0]))
	{
		send_response(cliSocket, ERR_ALREADYREGISTRED, info);
		printf("You have already registered!\n");
		// ERROR : ALREADY REGISTERED
	}
	else
	{
		memset(info->username, 0, maxUserLen);
		memset(info->name, 0, maxNameLen);
		for (int i=0; i<userLen; i++)
		{
			info->username[i] = username[i];
		}
		if (userOverflow)
			info->username[userLen-1] = '\0';
		for (int i=0; i<nameLen; i++)
		{
			info->name[i] = name[i];
		}
		if (nameOverflow)
			info->name[nameLen-1] = '\0';
		if (info->nickname[0])
		{
			list_append(userList, info);
			// sort list
			send_response(cliSocket, RPL_WELCOME, info);
			printf("Welcome to the server!\n");
			// Welcome message
		}
	}

	printf("User struct says USERNAME is %s\n", info->username);
	printf("User struct says NAME is %s\n", info->name);
}

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
			printf("Invalid command.\n");
			return -1;
	}
	return 1;
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

	list_t userList;
	list_init(&userList);
	list_attributes_copy(&userList, user_info_size, 1);

	while(1)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
		int msgLen = 512;
                int nbytes;
		char inputBuf[msgLen];
                char **commandList;

		memset(inputBuf, 0, msgLen);
		commandList = (char **) malloc(COMMANDNUM*sizeof(char **));
		command_init(commandList); // Initialize command array
		userInfo info = {{ 0 }};

		printf("First command: %s, Seccond command: %s\n", 
			commandList[0], commandList[1]); //remove later


		while( (nbytes = recv(clientSocket, inputBuf, msgLen, 0)) )
		{
			//handles case if string is too long
			if ((inputBuf[msgLen-1]) && (inputBuf[msgLen-1]!='\n'))
			{
				inputBuf[msgLen-2]='\r';
				inputBuf[msgLen-1]='\n'; //should a message be saved if it is cut off?
			}
			printf("I got your message! It is: %s", inputBuf); //remove later
			char **argList;
			int maxArgs = 15;
			argList = (char **) malloc((1+maxArgs)*sizeof(char **));
			int argNum = parser(inputBuf, nbytes, argList);
			printf("Number of words:%d\n", argNum); //remove later 


			int commandNum = command_search(argList[0], commandList);
			run_command(commandNum, argList, argNum, &info, &userList, clientSocket);

			if (list_get_at(&userList, 0)){			
				printf("In list: User has nickname: %s, name: %s, username: %s.\n", ((userInfo *)list_get_at(&userList, 0))->nickname,			
				((userInfo *)list_get_at(&userList, 0))->name,
				((userInfo *)list_get_at(&userList, 0))->username);
			}


                        if (list_get_at(&userList, 1)){
                                printf("In list: User has nickname: %s, name: %s, username: %s.\n", ((userInfo *)list_get_at(&userList, 1))->nickname,          
                                ((userInfo *)list_get_at(&userList, 1))->name,
                                ((userInfo *)list_get_at(&userList, 1))->username);
                        }


                        if (list_get_at(&userList, 2)){
                                printf("In list: User has nickname: %s, name: %s, username: %s.\n", ((userInfo *)list_get_at(&userList, 2))->nickname,          
                                ((userInfo *)list_get_at(&userList, 2))->name,
                                ((userInfo *)list_get_at(&userList, 2))->username);
                        }


			free(argList);

			memset(inputBuf, 0, msgLen); //clear buffer
		}
	}

	close(serverSocket);


	return 0;
}

