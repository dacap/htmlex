/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001 by David A. Capello
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
#include "macros.h"

struct _macros macros[MAX_MACROS];
int nmacros = 0;
unsigned char macros_table[256];

void remove_macros(void)
{
  int c;
  for (c=0; c<nmacros; c++) {
    if (macros[c].name) {
      free(macros[c].name);
      macros[c].name = NULL;
    }
    if (macros[c].value) {
      free(macros[c].value);
      macros[c].value = NULL;
    }
  }
  nmacros = 0;
}


