/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David A. Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __MACROS_H__
#define __MACROS_H__

struct MACRO;
struct MACRO_FUNC;
struct MACRO_LIST;

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

extern struct MACRO_LIST *global_macros;
extern struct MACRO_LIST *function_macros[MAX_MACRO_SPACES];
extern int nfunction_macros;

MACRO *new_macro (int type, char *name, char *value);
MACRO_LIST *new_macro_list (void);
void free_macro_list (MACRO_LIST * list);

void add_macro (MACRO_LIST * list, MACRO * macro, int sort);
int remove_macro (MACRO_LIST * list, const char *name);
MACRO *get_macro (MACRO_LIST * list, const char *name);
void modify_macro (MACRO * macro, char *value);

char *replace_by_macro (MACRO_LIST * list, char *buf, int *length);
char *function_macro (MACRO_LIST * list, char *tag);

#endif				/* __MACROS_H__ */
