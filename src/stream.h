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

#ifndef HTMLEX_STREAM_H
#define HTMLEX_STREAM_H

#include <stdio.h>



#define STREAM_SEEK_SET 0
#define STREAM_SEEK_CUR 1
#define STREAM_SEEK_END 2



typedef struct STREAM
{
  void *data;
  void *(*alloc)(FILE *file);
  void (*free)(void *data);
  int (*put)(void *data, int c);
  int (*get)(void *data);
  int (*seek)(void *data, int offset, int mode);
  int (*tell)(void *data);
  int (*eof)(void *data);
} STREAM;



STREAM *stream_open(FILE *file);
void stream_close(STREAM *stream);

int stream_putc(STREAM *stream, int c);
int stream_getc(STREAM *stream);
int stream_puts(STREAM *stream, const char *s);
char *stream_gets(STREAM *stream, char *s, int maxlength);
int stream_seek(STREAM *stream, int offset, int mode);
int stream_tell(STREAM *stream);
int stream_eof(STREAM *stream);

char *stream_buffer(STREAM *stream);



#endif /* not HTMLEX_STREAM_H */

