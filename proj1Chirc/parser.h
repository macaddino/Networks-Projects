/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Parser functions
 *
 */

#ifndef PARSER_H_
#define PARSER_H_

int break_commands(char *input, int nbytes, char **argList);
int parser(char *input,  int nbytes, char **argList);

#endif /* PARSER_H_ */
