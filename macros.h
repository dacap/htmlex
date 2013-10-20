/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David Capello
 *
 * This source file is distributed under MIT license,
 * please read LICENSE.txt for more information.
 */

#ifndef __MACROS_H__
#define __MACROS_H__

enum {
  NORMAL_MACRO,			/* normal macro */
  FUNCTIONAL_MACRO,		/* accessible functional macro */
  UNAVAILABLE_MACRO,		/* unavailable functional macro */
};

typedef struct MACRO {
  unsigned int type:2;
  char *name;
  char *value;
  void *data;
} MACRO;

typedef struct MACRO_LIST {
  int max_macros;
  int num_macros;
  MACRO *macros;
} MACRO_LIST;

#define MAX_MACRO_SPACES 1024

extern MACRO_LIST *macros_space[MAX_MACRO_SPACES];
extern int nmacros_space;

MACRO *new_macro (int type, char *name, char *value);
MACRO_LIST *new_macro_list (void);
void free_macro_list (MACRO_LIST *list);

void add_macro (MACRO_LIST *list, MACRO *macro, int sort);
int remove_macro (MACRO_LIST *list, const char *name);
MACRO *get_macro (MACRO_LIST *list, const char *name);
void modify_macro (MACRO *macro, char *value);

char *replace_by_macro (MACRO_LIST *list, char *buf, int *length);
char *function_macro (MACRO_LIST *list, char *tag);

#endif				/* __MACROS_H__ */
