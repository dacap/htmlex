/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David Capello
 *
 * This source file is distributed under MIT license,
 * please read LICENSE.txt for more information.
 */

#include <stdlib.h>
#include <string.h>
#include "htmlex.h"
#include "macros.h"

MACRO_LIST *macros_space[MAX_MACRO_SPACES];
int nmacros_space = 0;

static unsigned int macros_table[256];
static unsigned int funcs_table[256];

static void push_space (void);
static void pop_space (void);

static void destroy_macro_data (MACRO *macro)
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

void free_macro_list (MACRO_LIST *list)
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

void add_macro (MACRO_LIST *list, MACRO *macro, int sort)
{
  int c = list->num_macros++;

  if (list->num_macros > list->max_macros) {
    list->max_macros++;
    list->macros = realloc (list->macros, sizeof (MACRO) * list->max_macros);
  }

  if (macro->type == NORMAL_MACRO)
    macros_table[(unsigned char)*macro->name]++;
  else
    funcs_table[(unsigned char)*macro->name]++;

  memcpy (list->macros+c, macro, sizeof (MACRO));
  free (macro);

  if (sort)
    qsort (list->macros, list->num_macros, sizeof (MACRO), sort_macros);
}

int remove_macro (MACRO_LIST *list, const char *name)
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

MACRO *get_macro (MACRO_LIST *list, const char *name)
{
  int c;

  for (c = 0; c < list->num_macros; c++)
    if (strcmp (name, list->macros[c].name) == 0)
      return list->macros+c;

  return NULL;
}

void modify_macro (MACRO *macro, char *value)
{
  free (macro->value);
  macro->value = value;
}

char *replace_by_macro (MACRO_LIST *list, char *buf, int *length)
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

char *function_macro (MACRO_LIST *list, char *tag)
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
	    MACRO_LIST *sub_macros = new_macro_list ();
	    char *replacement, *tok;
	    char *holder = NULL;
	    MACRO *macro;
	    int argc = 0;

	    list->macros[c].type = UNAVAILABLE_MACRO;

	    for (tok = own_strtok (tag + 2, &holder),
		 tok = own_strtok (NULL, &holder); tok;
		 tok = own_strtok (NULL, &holder)) {
	      if (argc < arg_list->num_macros) {
		macro = new_macro (NORMAL_MACRO,
				   strdup (arg_list->macros[argc].name),
				   process_text (tok));

		add_macro (sub_macros, macro, TRUE);
	      }
	      argc++;
	    }

	    for (; argc < arg_list->num_macros; argc++) {
	      macro = new_macro (NORMAL_MACRO,
				 strdup (arg_list->macros[argc].name),
				 strdup (""));

	      add_macro (sub_macros, macro, TRUE);
	    }

	    push_space ();
	    macros_space[0] = sub_macros;
	    replacement = process_text (list->macros[c].value);
	    pop_space ();

	    free_macro_list (sub_macros);

	    list->macros[c].type = FUNCTIONAL_MACRO;

	    return replacement;
	  }
	}
      }
    }
  }

  return NULL;
}

static void push_space (void)
{
  memmove (macros_space + 1, macros_space, sizeof (MACRO_LIST *) * nmacros_space);
  nmacros_space++;
}

static void pop_space (void)
{
  nmacros_space--;
  memmove (macros_space, macros_space + 1, sizeof (MACRO_LIST *) * nmacros_space);
}
