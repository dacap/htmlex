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

#ifndef HTMLEX_TAG_H
#define HTMLEX_TAG_H


                               /* el valor de la propiedad... */
#define PROPERTY_FLAG_TAGS   1 /* ...puede tener etiquetas */
#define PROPERTY_FLAG_MACROS 2 /* ...puede tener macros */
#define PROPERTY_FLAG_EXPR   4 /* ...representa una expresi¢n */



typedef struct PROPERTY
{
  char *name;
  char *value;
  int flags;
} PROPERTY;



typedef struct TAG
{
  char *name;
  int nprop;
  PROPERTY *prop;
} TAG;



TAG *create_tag();
void destroy_tag(TAG *tag);

PROPERTY *add_property(TAG *tag);
PROPERTY *get_property(TAG *tag, const char *name);

TAG *tag_from_string(const char *_s);



#endif /* not HTMLEX_TAG_H */


