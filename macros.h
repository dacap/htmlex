/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002 by David A. Capello
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

#ifndef __MACROS_H__
#define __MACROS_H__

#define MAX_MACROS 1024

struct _macros
{
  char *name;
  char *value;
};

extern struct _macros macros[MAX_MACROS];
extern int nmacros;
extern unsigned char macros_table[256];

void remove_macros(void);

#endif /* __MACROS_H__ */

