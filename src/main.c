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
		if (input[n]==':')
		{
			int argSize = nbytes-start_cpy;
			argList[array_index] = (char *) malloc(argSize);
			memcpy(argList[array_index], input+start_cpy, argSize);
			argList[array_index][argSize] = '\0';
			array_index++;
			break;
		}
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

/* user_info_size:
 * Returns size of the userInfo struct.
 */
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

/* send_response:
 * Given the client socket, the response code, and a userInfo struct,
 * sends an appropriate response to the client.
 * Returns 0 upon success.
 */
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
	}
	else if (!(isFirstNick))
	{
		printf("Check to see if list has changed!\n");
		// update list CHECK TO SEE IF ALREADY UPDATED!!
	}

	printf("User struct says NICK is %s\n", info->nickname);

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
		}
	}

	printf("User struct says USERNAME is %s\n", info->username);
	printf("User struct says NAME is %s\n", info->name);
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
			printf("Invalid command.\n");
			return -1;
	}
	return 1;
}


int break_commands(char *input, int nbytes, char **argList)
{
	int array_index=0;
	int start_cpy=0;
	int n;
	for (n=0; n<nbytes; n++)
	{
		if ((input[n] == '\n') && (n>0) && (input[n-1] == '\r'))
		{
			int argSize = n-start_cpy;
			argList[array_index] = (char *) malloc(argSize);
			memcpy(argList[array_index], input+start_cpy, argSize);
			array_index++;
			start_cpy = n+1;
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

	list_t userList;
	list_init(&userList);
	list_attributes_copy(&userList, user_info_size, 1);

	while(1)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
		int msgLen = 512;
                int nbytes;
		int n;
		char inputBuf[msgLen];
		char buildBuf[msgLen];
                char **commandList;
		int buildLen = 0;

		memset(inputBuf, 0, msgLen);
		commandList = (char **) malloc(COMMANDNUM*sizeof(char **));
		command_init(commandList); // Initialize command array
		userInfo info = {{ 0 }};

		printf("First command: %s, Seccond command: %s\n", 
			commandList[0], commandList[1]); //remove later


		while( (nbytes = recv(clientSocket, inputBuf, msgLen, 0)) )
		{
			memcpy(buildBuf+buildLen, inputBuf, nbytes); 
			buildLen = buildLen + nbytes;
			
			printf("I got your message! It is: %s", inputBuf); //remove later
	
			//handles case if string is too long
			if ((buildBuf[msgLen-1]) && (buildBuf[msgLen-1]!='\n'))
			{
				buildBuf[msgLen-2]='\r';
				buildBuf[msgLen-1]='\n'; //should a message be saved if it is cut off?
			}
			else if (buildBuf[nbytes-1]=='\n' && nbytes>1 && buildBuf[nbytes-2]=='\r')
			{
				char **argList;
				int maxArgs = 15;
				argList = (char **) malloc((1+maxArgs)*sizeof(char **));
				char **cmndList;
				cmndList = (char **) malloc(msgLen*sizeof(char **));
				int numCmnds = break_commands(buildBuf, buildLen, cmndList);
				printf("Number of commands:%d\n", numCmnds);

				for (n=0; n<numCmnds; n++)
				{
					int argNum = parser(cmndList[n], nbytes, argList);
					printf("Number of words:%d\n", argNum); //remove later 
					int commandNum = command_search(argList[0], commandList);
					run_command(commandNum, argList, argNum, &info, &userList, clientSocket);
				}


				
				if (list_get_at(&userList, 0)){			
					printf("User struct nickname: %s\n", info.nickname);
					printf("User struct name: %s\n", info.name);
					printf("User struct username: %s\n", info.username);
					printf("In list: User has nickname: %s\n", ((userInfo *)list_get_at(&userList, 0))->nickname);
                                        printf("In list: User has name: %s\n", ((userInfo *)list_get_at(&userList, 0))->name);
                                        printf("In list: User has username: %s\n", ((userInfo *)list_get_at(&userList, 0))->username);

				}

				free(argList);
				buildLen = 0;
				memset(inputBuf, 0, msgLen); //clear buffer
			}
		}
	}

	close(serverSocket);


	return 0;
}

