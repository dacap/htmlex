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

#ifndef __STREAM_H__
#define __STREAM_H__

#define _IO_STREAM_TYPE_FILE     0
#define _IO_STREAM_TYPE_MEMORY   1

typedef struct _IO_STREAM STREAM;

struct _IO_STREAM
{
  int type;
  void *data;
};

STREAM *sopen(const char *filename, const char *mode);
STREAM *sfile(void *file);
char *sbuffer(STREAM *stream);
int sclose(STREAM *stream);
int seof(STREAM *stream);
int sflush(STREAM *stream);
int sgetc(STREAM *stream);
char *sgets(char *s, int size, STREAM *stream);
int sputc(int c, STREAM *stream);
int sputs(const char *s, STREAM *stream);
int sseek(STREAM *stream, int offset, int whence);
int stell(STREAM *stream);

#endif /* __STREAM_H__ */

