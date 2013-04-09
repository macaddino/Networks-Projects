/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Server Response functions
 *
 */

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "structures.h"

int send_response(int clientSocket, char * response, userInfo * info);

#endif /* RESPONSE_H_ */
