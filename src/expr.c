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
#include <stdlib.h>
#include <ctype.h>

#include "string.h"
#include "memory.h"



/* convierte un entero en una cadena con el n£mero representado decimal */
static char *dtos(int d)
{
  char buf[32];

  sprintf(buf, "%d", d);

  return strdup_ex(buf);
}



/* evalua una expresi¢n en una cadena de caracteres, notar  de que
   retorna una nueva cadena en memoria, por lo que la misma deber 
   ser liberada mediante free_ex(), adem s, la cadena de entrada
   (`s') es modificada; esta funci¢n soporta:
   NULL             = NULL
   ""               = NULL
   "text"           = "text"
   "text == text"   = "1"
   "text != text"   = "0"
   "0 == 0"         = "1"
   "0 != 0"         = "0"
   "8 + 2"          = "10"
   "8 - 2"          = "6"
   "2 * 6"          = "12"
   "4 / 2"          = "2"
*/
char *eval_expr(char *s)
{
  char *copy = strdup_ex(s);
  char *s1, *s2, *ope;

  /* si la cadena contiene texto */
  if ((s) && (*s)) {
    /* la primer cadena comienza desde el primer caracter de `s' */
    s1 = s;

    /* buscar por el primer espacio */
    ope = strchr_ex(s, " ");

    /* si no se encuentra un espacio, significa que s1 es un n£mero o
       simplemente una cadena de caracteres donde no se especific¢ un
       operador */
    if (!ope) {
      /* devolver la copia de la cadena y listo */
      return copy;
    }
    /* si se encontr¢ un espacio */
    else {
      /* colocar NULL para indicar el final de `s1' y avanzar un lugar
         el cual ser  el comienzo del operador */
      *(ope++) = 0;

      /* buscar por el pr¢ximo componente de la operaci¢n */
      s2 = strchr_ex(ope, " ");

      /* si se encontr¢ otro espacio */
      if (s2) {
        /* colocar NULL para indicar el final de `ope' y avanzar un lugar
           el cual ser  el comienzo del segundo elemento (s2) */
        *(s2++) = 0;
      }
      /* si no se encontr¢ otro espacio, asignarle a s2 una cadena de
         longitud cero (0) */
      else {
        s2 = "";
      }

      /* ahora ya tenemos el operador y los dos componentes
         a comparar listos para procesar... */

      /* comparar si las dos cadenas son iguales */
      if (strcmp_ex(ope, "==") == 0) {
        free_ex(copy);
        if (isdigit_ex(s1) && isdigit_ex(s2))
          return dtos(strtol(s1, NULL, 0) == strtol(s2, NULL, 0));
        else
          return dtos(strcmp_ex(s1, s2) == 0);
      }
      /* comparar si las dos cadenas son diferentes */
      else if (strcmp_ex(ope, "!=") == 0) {
        free_ex(copy);
        if (isdigit_ex(s1) && isdigit_ex(s2))
          return dtos(strtol(s1, NULL, 0) != strtol(s2, NULL, 0));
        else
          return dtos(strcmp_ex(s1, s2) != 0);
      }
      /* suma de n£meros */
      else if (strcmp_ex(ope, "+") == 0) {
        if (isdigit_ex(s1) && isdigit_ex(s2)) {
          free_ex(copy);
          return dtos(strtol(s1, NULL, 0) + strtol(s2, NULL, 0));
        }
      }
      /* resta de n£meros */
      else if (strcmp_ex(ope, "-") == 0) {
        if (isdigit_ex(s1) && isdigit_ex(s2)) {
          free_ex(copy);
          return dtos(strtol(s1, NULL, 0) - strtol(s2, NULL, 0));
        }
      }
      /* multiplicaci¢n de n£meros */
      else if (strcmp_ex(ope, "*") == 0) {
        if (isdigit_ex(s1) && isdigit_ex(s2)) {
          free_ex(copy);
          return dtos(strtol(s1, NULL, 0) * strtol(s2, NULL, 0));
        }
      }
      /* divisi¢n de n£meros */
      else if (strcmp_ex(ope, "/") == 0) {
        if (isdigit_ex(s1) && isdigit_ex(s2)) {
          int div = strtol(s2, NULL, 0);
          if (div != 0) {
            free_ex(copy);
            return dtos(strtol(s1, NULL, 0) / div);
          }
        }
      }
    }
  }

  /* devolver la copia de la cadena */
  return copy;
}


