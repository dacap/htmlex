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

#include <stdio.h>
#include <ctype.h>

#include "string.h"
#include "memory.h"



/* retorna la longitud de la cadena de caracteres `s' */
int strlen_ex(const char *s)
{
  const char *end = s;

  if (end)
    while (*end)
      end++;

  return end - s;
}



/* copia la cadena s2 en s1 */
char *strcpy_ex(char *s1, const char *s2)
{
  char *ret = s1;

  while (*s2)
    *(s1++) = *(s2++);

  *s1 = 0;
  return ret;
}



/* devuelve la direcci¢n donde se encontr¢ el primer caracter en `s1'
   de los que est n especificados en `s2' */
char *strchr_ex(const char *s1, const char *s2)
{
  const char *s;

  for (; *s1; s1++) {
    for (s=s2; *s; s++)
      if (*s1 == *s)
        return (char *)s1;
  }

  return NULL;
}



/* al igual que strcmp(), compara dos cadenas de caracteres completas
   entre s¡ y retorna 0 si son iguales */
int strcmp_ex(const char *s1, const char *s2)
{
  int c1, c2;

  for (;;) {
    c1 = *(s1++);
    c2 = *(s2++);

    if (c1 != c2)
      return c1 - c2;

    if (!c1)
      break;
  }

  return 0;
}



/* igual que strcmp_ex(), solamente que se puede especificar un l¡mite
   de caracteres hasta donde comparar */
int strncmp_ex(const char *s1, const char *s2, int max)
{
  int c, c1, c2;

  for (c=0; c<max; c++) {
    c1 = *(s1++);
    c2 = *(s2++);

    if (c1 != c2)
      return c1 - c2;

    if (!c1)
      break;
  }

  return 0;
}



/* igual que strncmp_ex(), solamente que el l¡mite de caracteres est 
   dado por la longitud de la cadena `s2', esto quiere decir que si
   la cadena `s2' se encuentra al inicio de `s1' (sin importar los
   caracteres que le sigan a `s1') esta funci¢n retorna 0 */
int strn2cmp_ex(const char *s1, const char *s2)
{
  return strncmp_ex(s1, s2, strlen_ex(s2));
}



/* compara igual que strcmp_ex(), solamente que ahora, sin importar el caso
   de las letras (may£sculas y min£sculas son iguales) */
int stricmp_ex(const char *s1, const char *s2)
{
  int c1, c2;

  for (;;) {
    c1 = tolower(*(s1++));
    c2 = tolower(*(s2++));

    if (c1 != c2)
      return c1 - c2;

    if (!c1)
      break;
  }

  return 0;
}



/* uni¢n de las funciones stricmp_ex() y strncmp_ex() */
int strnicmp_ex(const char *s1, const char *s2, int max)
{
  int c, c1, c2;

  for (c=0; c<max; c++) {
    c1 = tolower(*(s1++));
    c2 = tolower(*(s2++));

    if (c1 != c2)
      return c1 - c2;

    if (!c1)
      break;
  }

  return 0;
}



/* uni¢n de las funciones stricmp_ex() y strn2cmp_ex() */
int strni2cmp_ex(const char *s1, const char *s2)
{
  return strnicmp_ex(s1, s2, strlen_ex(s2));
}



/* realiza una copia de la cadena de origen `s' */
char *strdup_ex(const char *s)
{
  char *s2;
  int len;

  len = strlen_ex(s) + 1;

  s2 = malloc_ex(sizeof(char) * len);
  if (s2) {
    if (s)
      memcpy_ex(s2, s, len);
    else
      *s2 = 0;
  }

  return s2;
}



/* a diferencia del strcat() ANSI, esta funci¢n contatena las dos cadenas
   en un nuevo espacio en memoria, por lo que el resultado deber  de
   ser liberado mediante free_ex(). (notese que tanto `s1' como `s2'
   son constantes) */
char *strcat_ex(const char *s1, const char *s2)
{
  char *s;

  s = (char *)malloc_ex(sizeof(char) * (strlen_ex(s1) + strlen_ex(s2) + 1));

  sprintf(s, "%s%s", s1, s2);
  return s;
}



/* si la cadena termina con un '\n', se lo quita */
char *chop_ex(char *s)
{
  int len;

  if ((s) && (*s)) {
    len = strlen_ex(s);
    if (s[len-1] == '\n')
      s[len-1] = 0;
  }

  return s;
}



/* devuelve no-cero si la cadena representa un d¡gito */
int isdigit_ex(const char *s)
{
  /* si tiene signo, saltearlo */
  if ((*s == '-') || (*s == '+'))
    s++;

  /* hexadecimal */
  if (strni2cmp_ex(s, "0x") == 0) {
    for (s=s+2; *s; s++)
      if (!isxdigit(*s))
        return 0;
  }
  /* octal */
  else if (*s == '0') {
    for (; *s; s++)
      if ((!isdigit(*s)) || (*s > '7'))
        return 0;
  }
  /* decimal */
  else {
    for (; *s; s++)
      if (!isdigit(*s))
        return 0;
  }

  /* es un n£mero, devolver un valor diferente a cero */
  return !0;
}


