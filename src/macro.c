/* HTMLex - an EXtra EXtension for HTML files
   Copyright (C) 2001 by David A. Capello

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "bool.h"
#include "macro.h"
#include "string.h"
#include "memory.h"



/* crea una nueva macro vac¡a */
MACRO *create_macro()
{
  MACRO *macro;

  macro = (MACRO *)malloc_ex(sizeof(MACRO));
  if (!macro)
    return NULL;

  macro->name = NULL;
  macro->value = NULL;
  macro->used = FALSE;
  macro->next = NULL;

  return macro;
}



/* borra una macro de la memoria */
void destroy_macro(MACRO *macro)
{
  if (macro) {
    if (macro->name)
      free_ex(macro->name);

    if (macro->value)
      free_ex(macro->value);

    free_ex(macro);
  }
}



/* agrega una macro a la lista, las va agregando seg£n longitud del nombre
   (las m s largas son las primeras) */
MACRO *insert_macro(MACRO **list, MACRO *macro)
{
  MACRO *iter, *prev;
  int len;

  if ((list) && (macro)) {
    len = strlen_ex(macro->name);

    for (iter=*list, prev=NULL; iter; prev=iter, iter=iter->next) {
      if (len >= strlen_ex(iter->name))
        break;
    }

    if (prev)
      prev->next = macro;
    else
      *list = macro;

    macro->next = iter;
  }

  return macro;
}



/* borra la macro indicada de la lista */
MACRO *remove_macro(MACRO **list, MACRO *macro)
{
  MACRO *iter, *prev;

  if ((list) && (macro)) {
    for (iter=*list, prev=NULL; iter; prev=iter, iter=iter->next) {
      if (iter == macro) {
        if (prev)
          prev->next = macro->next;
        else
          *list = macro->next;
        break;
      }
    }
  }

  return macro;
}



/* obtiene una macro de la lista por su nombre */
MACRO *get_macro(MACRO **list, const char *name, int len)
{
  MACRO *iter = NULL;

  if ((list) && (*list)) {
    if (len <= 0) {
      for (iter=*list; iter; iter=iter->next)
        if (strcmp_ex(iter->name, name) == 0)
          break;
    }
    else {
      for (iter=*list; iter; iter=iter->next)
        if (strncmp_ex(iter->name, name, len) == 0)
          break;
    }
  }

  return iter;
}




