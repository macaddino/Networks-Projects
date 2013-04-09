/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Custom SimCList Functions
 *
 */
#include <stdio.h>
#include "listfxns.h"


/* user_info_size:
 * Returns size of the userInfo struct.
 */
size_t user_info_size(const void *el)
{
        return sizeof(userInfo);
}

