/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David Capello
 *
 * This file is released under the terms of the MIT license.
 * Read LICENSE.txt for more information.
 */

#ifndef __STREAM_H__
#define __STREAM_H__

#define STREAM_TYPE_FILE     0
#define STREAM_TYPE_MEMORY   1

typedef struct STREAM {
  int type;
  void *data;
} STREAM;

STREAM *stopen (const char *filename, const char *mode);
STREAM *stfile (void *file);
char *stbuffer (STREAM *stream);
int stclose (STREAM *stream);
int steof (STREAM *stream);
int stflush (STREAM *stream);
int stgetc (STREAM *stream);
char *stgets (char *s, int size, STREAM *stream);
int stputc (int c, STREAM *stream);
int stputs (const char *s, STREAM *stream);
int stseek (STREAM *stream, int offset, int whence);
int sttell (STREAM *stream);

#endif				/* __STREAM_H__ */
