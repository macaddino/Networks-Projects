/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Parser Functions
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* parser:
 * Given a command string, its number of bytes,
 * and an empty array of strings, fills the string array with
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
		// all chars following a colon are stored as only one string
                if (input[n]==':')
                {
                        int argSize = nbytes-start_cpy;
                        argList[array_index] = (char *) malloc(argSize);
                        memcpy(argList[array_index], input+start_cpy, argSize);
                        argList[array_index][argSize] = '\0';
                        array_index++;
                        break;
                }
		// argument ends if followed by a space or if at end of input
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

/* break_commands:
 * Given a string of one or more commands, their total number
 * of bytes, and an empty array of strings, stores each command
 * which ends with "\r\n" as a separate string in string array.
 * Returns the total number of parsed commands.
 */
int break_commands(char *input, int nbytes, char **cmndList)
{
        int array_index=0;
        int start_cpy=0;
        int n;
        for (n=0; n<nbytes; n++)
        {
                if ((input[n] == '\n') && (n>0) && (input[n-1] == '\r'))
                {
                        int cmndSize = n-start_cpy;
                        cmndList[array_index] = (char *) malloc(cmndSize);
                        memcpy(cmndList[array_index], input+start_cpy, cmndSize);
                        array_index++;
                        start_cpy = n+1;
                }
        }
        return array_index;
}

