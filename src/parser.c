/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Parser Functions
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


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
                if ((input[n] == ' ') || (n == nbytes-1))
                {
                        int argSize = n-start_cpy;
                        argList[array_index] = (char *) malloc(argSize+1);
                        memcpy(argList[array_index], input+start_cpy, argSize);
                        argList[array_index][argSize] = '\0';
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

