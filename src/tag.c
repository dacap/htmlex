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

#include "tag.h"
#include "memory.h"
#include "string.h"



TAG *create_tag()
{
  TAG *tag;

  tag = (TAG *)malloc_ex(sizeof(TAG));
  if (!tag)
    return NULL;

  tag->name = NULL;
  tag->nprop = 0;
  tag->prop = NULL;

  return tag;
}



void destroy_tag(TAG *tag)
{
  int i;

  if (tag) {
    if (tag->prop) {
      for (i=0; i<tag->nprop; i++) {
        if (tag->prop[i].name)
          free_ex(tag->prop[i].name);

        if (tag->prop[i].value)
          free_ex(tag->prop[i].value);
      }
      free_ex(tag->prop);
    }

    if (tag->name)
      free_ex(tag->name);

    free_ex(tag);
  }
}



/* agrega una nueva propiedad a la etiqueta y retorna su puntero para
   poder modificarla */
PROPERTY *add_property(TAG *tag)
{
  int c;

  if (tag) {
    c = tag->nprop++;
    tag->prop = (PROPERTY *)realloc_ex(tag->prop, sizeof(PROPERTY) * tag->nprop);
    if (!tag->prop) {
      tag->nprop = 0;
      return NULL;
    }

    tag->prop[c].name = NULL;
    tag->prop[c].value = NULL;
    tag->prop[c].flags = 0;

    return tag->prop+c;
  }

  return NULL;
}



/* devolver la propiedad que tenga el nombre especificado */
PROPERTY *get_property(TAG *tag, const char *name)
{
  int i;

  /* buscar por todas las propiedades */
  for (i=0; i<tag->nprop; i++) {
    /* comparar los dos nombres */
    if (stricmp_ex(name, tag->prop[i].name) == 0)
      /* si son iguales retornar esta propiedad */
      return tag->prop+i;
  }

  /* no se encontr¢ la propiedad con dicho nombre */
  return NULL;
}



/* normaliza una cadena (`type' puede ser '\'' o '\"'):
   - quita las comillas en el caso de que `type' indique las tiene;
   - en caso de que tenga comillas, esta funci¢n junta caracteres
     consecutivos como `\' y `n' a un verdadero '\n';
*/
static char *normalize_string(char *s, int type)
{
  char *s1 = s, *s2;

  for (; *s; s++) {
retry:;
    /* si se encuentra una barra invertida `\' */
    if (*s == '\\') {
      /* en los dos tipos de comilla */
      if ((type == '\'') || (type == '\"')) {
        /* ver si se trata de una concatenaci¢n con la l¡nea de abajo */
        for (s2=s+1; *s2; s2++) {
          if ((*s2 != ' ') && (*s2 != '\t')) {
            if (*s2 == '\n') {
              memmove_ex(s, s2+1, strlen_ex(s2+1)+1);
              goto retry;
            }
            else
              break;
          }
        }
      }

      /* en comilla simple */
      if (type == '\'') {
        if (*(s+1) == '\'')
          memmove_ex(s, s+1, strlen_ex(s+1)+1);
      }
      /* en comilla doble */
      else if (type == '\"') {
        memmove_ex(s, s+1, strlen_ex(s+1)+1);
        switch (*s) {
          case 'n': *s = '\n'; break;
          case 't': *s = '\t'; break;
        }
      }
    }
  }

  return s1;
}



/* convirte una cadena "tag prop1=value prop2=value" a una estructura TAG */
TAG *tag_from_string(const char *_s)
{
  TAG *tag;
  PROPERTY *prop;
  char *s, *t;
  int c, d;

  /* alojar memoria para la etiqueta */
  tag = create_tag();

  /* leer nombre de la etiqueta... */

  /* buscar por el primer caracter de control (espacio, tabulador
     o nueva l¡nea) */
  s = strchr_ex(_s, " \t\n");

  /* solamente tiene nombre ya que comienza y termina con letras, sin
     espacios intermedios */
  if (!s) {
    tag->name = strdup_ex(_s);
  }
  /* tiene nombre y tal vez propiedades, ya que se podr¡a tratar
     de algo como esto: "tag  " */
  else {
    c = *s;
    *s = 0;
    tag->name = strdup_ex(_s);
    *s = c;

    /* leer las propiedades */
    for (; (*s); ) {
      /* si no es caracter de control */
      if ((*s != ' ') && (*s != '\t') && (*s != '\n')) {
        /* insertar una nueva propiedad en la etiqueta */
        prop = add_property(tag);

        /* leer el nombre de la propiedad */
        for (t=s; (*t) && (*t != '=') && (*t != '\t') &&
                          (*t != ' ') && (*t != '\n'); t++);

        c = *t;
        *t = 0;
        prop->name = strdup_ex(s);
        *t = c;

        /* propiedad con valor nulo */
        if (c != '=') {
          /* saltearse el nombre de la propiedad */
          s = t;

          /* la propiedad tiene valor totalmente nulo */
          prop->value = NULL;
        }
        /* leer el valor de la propiedad */
        else {
          s = ++t; /* saltearse el '=' y desde ah¡ comienza el valor */

          /* si el valor comienza con " o ', el valor ahora debe comenzar
             un caracter m s adelante */
          d = *s;
          if ((d == '\"') || (d == '\''))
            s++;

          /* leer todo el valor hasta el final */
          for (t=s; (*t); t++) {
            if ((d == '\"') || (d == '\'')) {
              if ((*t == d) && (*(t-1) != '\\'))
                break;
            }
            else {
              if ((*t == ' ') || (*t == '\t') || (*t == '\n'))
                break;
            }
          }

          c = *t;
          *t = 0;
          prop->value = normalize_string(strdup_ex(s), d);
          *t = c;

          /* colocar a `s' en la posici¢n correcta para pasar a leer la
             pr¢xima propiedad */
          s = t;
          if ((d == '\"') || (d == '\''))
            s++;
        }
      }
      else {
        s++;
      }
    }
  }

  /* devolver la etiqueta lista para utilizar */
  return tag;
}

