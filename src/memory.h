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

#ifndef HTMLEX_MEMORY_H
#define HTMLEX_MEMORY_H



#ifndef NULL
  #define NULL  0
#endif



int get_memory_used();

void *malloc_ex(int size);
void *realloc_ex(void *ptr, int size);
void free_ex(void *ptr);

void *memset_ex(void *buffer, int ch, int num);
void *memcpy_ex(void *dest, const void *src, int num);
void *memmove_ex(void *dest, const void *src, int num);



#endif /* not HTMLEX_MEMORY_H */

