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

#ifndef HTMLEX_STRING_H
#define HTMLEX_STRING_H



int strlen_ex(const char *s);
char *strcpy_ex(char *s1, const char *s2);
char *strchr_ex(const char *s1, const char *s2);
int strcmp_ex(const char *s1, const char *s2);
int strncmp_ex(const char *s1, const char *s2, int max);
int strn2cmp_ex(const char *s1, const char *s2);
int stricmp_ex(const char *s1, const char *s2);
int strnicmp_ex(const char *s1, const char *s2, int max);
int strni2cmp_ex(const char *s1, const char *s2);
char *strdup_ex(const char *s);
char *strcat_ex(const char *s1, const char *s2);
char *chop_ex(char *s);

int isdigit_ex(const char *s);



#endif /* not HTMLEX_STRING_H */

