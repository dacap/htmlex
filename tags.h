/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David Capello
 *
 */

#ifndef __TAGS_H__
#define __TAGS_H__

typedef struct TAG {
  char *name;
  char *(*proc) (int argc, char *argv[]);
  int if_tag:1;
} TAG;

extern TAG *tags;
extern int ntags;

#endif				/* __TAGS_H__ */
