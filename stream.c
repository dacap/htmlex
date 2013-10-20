/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David Capello
 *
 * This source file is distributed under MIT license,
 * please read LICENSE.txt for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"

#define smemdata ((STREAM_MEMORY *)stream->data)

typedef struct STREAM_MEMORY {
  char *buf;			/* buffer for the characters */
  int size;			/* real size of the buffer */
  int end;			/* index of the last character used in the buffer */
  int pos;			/* the position for read data */
} STREAM_MEMORY;

STREAM *stopen (const char *filename, const char *mode)
{
  STREAM *stream = malloc (sizeof (STREAM));
  if (stream) {
    if (filename) {
      stream->type = STREAM_TYPE_FILE;
      stream->data = fopen (filename, mode);
      if (!stream->data) {
	free (stream);
	stream = NULL;
      }
    }
    else {
      stream->type = STREAM_TYPE_MEMORY;
      stream->data = malloc (sizeof (STREAM_MEMORY));
      if (!stream->data) {
	free (stream);
	stream = NULL;
      }
      else {
	smemdata->buf = NULL;
	smemdata->size = 0;
	smemdata->end = 0;
	smemdata->pos = 0;
      }
    }
  }
  return stream;
}

STREAM *stfile (void *file)
{
  STREAM *stream = malloc (sizeof (STREAM));
  if (stream) {
    stream->type = STREAM_TYPE_FILE;
    stream->data = file;
  }
  return stream;
}

char *stbuffer (STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_MEMORY) {
      char *buf = smemdata->buf;
      if (buf)
	buf[smemdata->end] = 0;
      free (stream->data);
      free (stream);
      return buf;
    }
  }
  return NULL;
}

int stclose (STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE) {
      int r;
      if ((stream->data != stdin) &&
	  (stream->data != stdout) && (stream->data != stderr))
	r = fclose (stream->data);
      else
	r = 0;
      free (stream);
      return r;
    }
    else if (stream->type == STREAM_TYPE_MEMORY) {
      if (stream->data) {
	if (smemdata->buf)
	  free (smemdata->buf);
	free (stream->data);
      }
      free (stream);
      return 0;
    }
  }
  return EOF;
}

int steof (STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return feof (stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY)
      return (smemdata->pos == smemdata->end);
  }
  return 1;
}

int stflush (STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return fflush (stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY)
      return 0;
  }
  return EOF;
}

int stgetc (STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return fgetc (stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY) {
      if (smemdata->pos < smemdata->end)
	return smemdata->buf[smemdata->pos++];
    }
  }
  return EOF;
}

char *stgets (char *s, int size, STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return fgets (s, size, stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY) {
      if (smemdata->pos == smemdata->end)
	return NULL;
      else {
	char *r = s;
	int c, i;
	for (i = 0; i < size; i++) {
	  if (smemdata->pos < smemdata->end) {
	    c = smemdata->buf[smemdata->pos++];
	    *(s++) = c;
	    if (c == '\n')
	      break;
	  }
	  else
	    break;
	}
	*s = 0;
	return r;
      }
    }
  }
  return NULL;
}

int stputc (int c, STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return fputc (c, stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY) {
      if (smemdata->end >= smemdata->size) {
	smemdata->size += 1024;
	smemdata->buf = realloc (smemdata->buf, smemdata->size);
	if (!smemdata->buf) {
	  smemdata->size = 0;
	  smemdata->end = 0;
	  smemdata->pos = 0;
	  return EOF;
	}
      }

      if (smemdata->pos < smemdata->end)
	memmove (smemdata->buf + smemdata->pos + 1,
		 smemdata->buf + smemdata->pos,
		 smemdata->end - smemdata->pos);

      smemdata->end++;
      return smemdata->buf[smemdata->pos++] = c;
    }
  }
  return EOF;
}

int stputs (const char *s, STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return fputs (s, stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY) {
      for (; *s; s++)
	if (stputc (*s, stream) == EOF)
	  return EOF;
      return 0;
    }
  }
  return EOF;
}

int stseek (STREAM *stream, int offset, int whence)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return fseek (stream->data, offset, whence);
    else if (stream->type == STREAM_TYPE_MEMORY) {
      switch (whence) {
	case SEEK_SET:
	  smemdata->pos = offset;
	  break;
	case SEEK_CUR:
	  smemdata->pos += offset;
	  break;
	case SEEK_END:
	  smemdata->pos = smemdata->end + offset;
	  break;
	default:
	  return -1;
      }
      if (smemdata->pos < 0)
	smemdata->pos = 0;
      else if (smemdata->pos > smemdata->end)
	smemdata->pos = smemdata->end;
      return 0;
    }
  }
  return -1;
}

int sttell (STREAM *stream)
{
  if (stream) {
    if (stream->type == STREAM_TYPE_FILE)
      return ftell (stream->data);
    else if (stream->type == STREAM_TYPE_MEMORY)
      return smemdata->pos;
  }
  return -1;
}
