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
#include <string.h>
#include <errno.h>

#include "bool.h"
#include "memory.h"



static int memory_count = 0;
static int memory_used = 0;



static void memory_check()
{
  if (memory_count != 0)
    fprintf(stderr, "Error: memory_count != 0: %d\n", memory_count);
}



int get_memory_used()
{
  return memory_used;
}



void *malloc_ex(int size)
{
  void *ptr;

  if (!memory_used)
    atexit(memory_check);

  memory_used += size;
  ptr = malloc(size);
  if (!ptr) {
    ptr = malloc(1);
    errno = ENOMEM;
  }

  memory_count++;
  return ptr;
}



void *realloc_ex(void *ptr, int size)
{
  if (!ptr)
    return malloc_ex(size);
  else {
    memory_used += size;
    ptr = realloc(ptr, size);
    if (!ptr) {
      ptr = malloc(1);
      errno = ENOMEM;
    }
    return ptr;
  }
}



void free_ex(void *ptr)
{
  if (ptr) {
    free(ptr);
    memory_count--;
  }
}



void *memset_ex(void *buffer, int ch, int num)
{
  if ((buffer) && (num > 0))
    return memset(buffer, ch, num);
  else
    return buffer;
}



void *memcpy_ex(void *dest, const void *src, int num)
{
  if ((dest) && (src) && (num > 0))
    return memcpy(dest, src, num);
  else
    return dest;
}



void *memmove_ex(void *dest, const void *src, int num)
{
  if ((dest) && (src) && (num > 0))
    return memmove(dest, src, num);
  else
    return dest;
}



