/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Aux Functions to create global lists maintained
 *  throughout a server runtime.
 *
 */
#include <stdio.h>
#include <string.h>
#include "listfxns.h"


/* user_info_size:
 * Returns size of the userInfo struct.
 */
size_t user_info_size(const void *el)
{
  return sizeof(userInfo);
}


int nick_comparator(const void *a, const void *b)
{
  userInfo *userInfoA = (userInfo *) a;
  userInfo *userInfoB = (userInfo *) b;
  char * nickA, * nickB;
  nickA = userInfoA->nickname;
  nickB = userInfoB->nickname;
  return strcmp(nickA, nickB);
}


int seeker(const void *el, const void ** nick)
{
  // let's assume el and key being always != NULL
  const userInfo *info = (userInfo *) el;
  if (!(strcmp(info->nickname, *(char **) nick)))
    return 1;
  return 0;
}

/* chan_info_size:
 * Returns size of the channelData struct.
 */
size_t chan_info_size(const void *el)
{
  return sizeof(channelData);
}

int chan_comparator(const void *a, const void *b)
{
  channelData *chanDataA = (channelData *) a;
  channelData *chanDataB = (channelData *) b;
  char * nameA, * nameB;
  nameA = chanDataA->name;
  nameB = chanDataB->name;
  return strcmp(nameA, nameB);
}

int chan_seeker(const void *el, const void ** name)
{
  // let's assume el and key being always != NULL
  const channelData *data = (channelData *) el;
  if (!(strcmp(data->name, *(char **) name)))
    return 1;
  return 0;
}

size_t chanmode_info_size(const void *el)
{
  return sizeof(forChannel);
}

int chanmode_comparator(const void *a, const void *b)
{
  forChannel *chanModeDataA = (forChannel *) a;
  forChannel *chanModeDataB = (forChannel *) b;
  char * nameA, * nameB;
  nameA = chanModeDataA->channelName;
  nameB = chanModeDataB->channelName;
  return strcmp(nameA, nameB);
}

int chanmode_seeker(const void *el, const void ** name)
{
  // let's assume el and key being always != NULL
  const forChannel *data = (forChannel *) el;
  if (!(strcmp(data->channelName, *(char **) name)))
    return 1;
  return 0;
}

