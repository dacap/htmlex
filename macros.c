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

#include <stdlib.h>
#include <string.h>
#include "htmlex.h"
#include "macros.h"

MACRO_LIST *global_macros = NULL;
MACRO_LIST *function_macros[MAX_MACRO_SPACES];
int nfunction_macros = 0;

static unsigned int macros_table[256];
static unsigned int funcs_table[256];

static void push_function_space (void);
static void pop_function_space (void);

static void destroy_macro_data (MACRO * macro)
{
  free (macro->name);
  free (macro->value);
  if (macro->type == FUNCTIONAL_MACRO)
    free_macro_list (macro->data);
}

MACRO *new_macro (int type, char *name, char *value)
{
  MACRO *macro;

  macro = malloc (sizeof (MACRO));
  if (!macro)
    return NULL;

  macro->type = type;
  macro->name = name;
  macro->value = value;

  if (type == FUNCTIONAL_MACRO)
    macro->data = new_macro_list ();
  else
    macro->data = NULL;

  return macro;
}

MACRO_LIST *new_macro_list (void)
{
  MACRO_LIST *list;

  list = malloc (sizeof (MACRO_LIST));
  if (!list)
    return NULL;

  list->max_macros = 0;
  list->num_macros = 0;
  list->macros = NULL;

  return list;
}

void free_macro_list (MACRO_LIST * list)
{
  int c;

  if (list->macros) {
    while (list->num_macros > 0)
      remove_macro (list, list->macros->name);

    free (list->macros);
  }

  free (list);
}

static int sort_macros (const void *e1, const void *e2)
{
  return strlen (((MACRO *) e2)->name) - strlen (((MACRO *) e1)->name);
}

void add_macro (MACRO_LIST * list, MACRO * macro, int sort)
{
  int c;

  c = list->num_macros++;

  if (list->num_macros > list->max_macros) {
    list->max_macros++;
    list->macros =
	realloc (list->macros, sizeof (MACRO) * list->max_macros);
  }

  if (macro->type == NORMAL_MACRO)
    macros_table[(unsigned char)*macro->name]++;
  else
    funcs_table[(unsigned char)*macro->name]++;

  list->macros[c] = *macro;

  free (macro);

  if (sort)
    qsort (list->macros, list->num_macros, sizeof (MACRO), sort_macros);
}

int remove_macro (MACRO_LIST * list, const char *name)
{
  int c;

  for (c = 0; c < list->num_macros; c++) {
    if (strcmp (name, list->macros[c].name) == 0) {
      if (list->macros[c].type == NORMAL_MACRO)
	macros_table[(unsigned char)*name]--;
      else
	funcs_table[(unsigned char)*name]--;

      destroy_macro_data (list->macros + c);

      for (list->num_macros--; c < list->num_macros; c++)
	list->macros[c] = list->macros[c + 1];
      return 0;
    }
  }

  return -1;
}

MACRO *get_macro (MACRO_LIST * list, const char *name)
{
  int c;

  for (c = 0; c < list->num_macros; c++)
    if (strcmp (name, list->macros[c].name) == 0)
      return list->macros+c;

  return NULL;
}

void modify_macro (MACRO * macro, char *value)
{
  free (macro->value);
  macro->value = value;
}

char *replace_by_macro (MACRO_LIST * list, char *buf, int *length)
{
  int c;

  /* this character is the start of a macro? */
  if (macros_table[(unsigned char)*buf]) {
    /* search to see if really it is a macro */
    for (c = 0; c < list->num_macros; c++) {
      if ((list->macros[c].type == NORMAL_MACRO) &&
	  (*buf == *list->macros[c].name)) {
	if (strncmp (buf,
		     list->macros[c].name,
		     strlen (list->macros[c].name)) == 0) {
	  log_printf (2, "macro found: \"%s\" -> \"%s\"\n",
		      list->macros[c].name, list->macros[c].value);
	  *length = strlen (list->macros[c].name);
	  return strdup (list->macros[c].value);
	}
      }
    }
  }

  *length = 0;
  return NULL;
}

char *function_macro (MACRO_LIST * list, char *tag)
{
  int c;

  /* this character is the start of a function? */
  if (funcs_table[(unsigned char)tag[1]]) {
    /* search to see if really it is a macro */
    for (c = 0; c < list->num_macros; c++) {
      /* functional macro and the first character of the function is
	 the same */
      if ((list->macros[c].type == FUNCTIONAL_MACRO) &&
	  (tag[1] == *list->macros[c].name)) {
	/* compare the entire names */
	if (strncmp (tag+1,
		     list->macros[c].name,
		     strlen (list->macros[c].name)) == 0) {
	  int x = tag[1 + strlen (list->macros[c].name)];
	  if (IS_BLANK (x) || (!x)) {
	    MACRO_LIST *arg_list = list->macros[c].data;
	    char *replacement, *tok;
	    char *holder = NULL;
	    MACRO *macro;
	    int argc = 0;

	    list->macros[c].type = UNAVAILABLE_MACRO;

	    for (tok = own_strtok (tag + 2, &holder),
		 tok = own_strtok (NULL, &holder); tok;
		 tok = own_strtok (NULL, &holder)) {
	      if (argc < arg_list->num_macros)
		modify_macro (arg_list->macros+argc, process_text (tok));

	      argc++;
	    }

	    for (; argc < arg_list->num_macros; argc++)
	      modify_macro (arg_list->macros+argc, strdup (""));

	    push_function_space ();
	    function_macros[0] = list->macros[c].data;

	    replacement = process_text (list->macros[c].value);

	    pop_function_space ();

	    list->macros[c].type = FUNCTIONAL_MACRO;

	    return replacement;
	  }
	}
      }
    }
  }

  return NULL;
}

static void push_function_space (void)
{
  if (nfunction_macros > 0)
    memmove (function_macros + 1, function_macros,
	     sizeof (MACRO_LIST *) * nfunction_macros);

  nfunction_macros++;
}

static void pop_function_space (void)
{
  nfunction_macros--;

  if (nfunction_macros > 0)
    memmove (function_macros, function_macros + 1,
	     sizeof (MACRO_LIST *) * nfunction_macros);
}
