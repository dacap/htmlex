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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stream.h"

#define _io_smemdata ((struct _IO_STREAM_MEMORY *)stream->data)

struct _IO_STREAM_MEMORY
{
  char *buf;      /* buffer for the characters */
  int size;       /* real size of the buffer */
  int end;        /* index of the last character used in the buffer */
  int pos;        /* the position for read data */
};

STREAM *stopen(const char *filename, const char *mode)
{
  STREAM *stream = malloc(sizeof(STREAM));
  if (stream) {
    if (filename) {
      stream->type = _IO_STREAM_TYPE_FILE;
      stream->data = fopen(filename, mode);
      if (!stream->data) {
        free(stream);
        stream = NULL;
      }
    }
    else {
      stream->type = _IO_STREAM_TYPE_MEMORY;
      stream->data = malloc(sizeof(struct _IO_STREAM_MEMORY));
      if (!stream->data) {
        free(stream);
        stream = NULL;
      }
      else {
        _io_smemdata->buf = NULL;
        _io_smemdata->size = 0;
        _io_smemdata->end = 0;
        _io_smemdata->pos = 0;
      }
    }
  }
  return stream;
}

STREAM *stfile(void *file)
{
  STREAM *stream = malloc(sizeof(STREAM));
  if (stream) {
    stream->type = _IO_STREAM_TYPE_FILE;
    stream->data = file;
  }
  return stream;
}

char *stbuffer(STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      char *buf = _io_smemdata->buf;
      if (buf)
        buf[_io_smemdata->end] = 0;
      free(stream->data);
      free(stream);
      return buf;
    }
  }
  return NULL;
}

int stclose(STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE) {
      int r;
      if ((stream->data != stdin) &&
          (stream->data != stdout) &&
          (stream->data != stderr))
        r = fclose(stream->data);
      else
        r = 0;
      free(stream);
      return r;
    }
    else if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      if (stream->data) {
        if (_io_smemdata->buf)
          free(_io_smemdata->buf);
        free(stream->data);
      }
      free(stream);
      return 0;
    }
  }
  return EOF;
}

int steof(STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return feof(stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY)
      return (_io_smemdata->pos == _io_smemdata->end);
  }
  return 1;
}

int stflush(STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return fflush(stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY)
      return 0;
  }
  return EOF;
}

int stgetc(STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return fgetc(stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      if (_io_smemdata->pos < _io_smemdata->end)
        return _io_smemdata->buf[_io_smemdata->pos++];
    }
  }
  return EOF;
}

char *stgets(char *s, int size, STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return fgets(s, size, stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      if (_io_smemdata->pos == _io_smemdata->end)
        return NULL;
      else {
        char *r = s;
        int c, i;
        for (i=0; i<size; i++) {
          if (_io_smemdata->pos < _io_smemdata->end) {
            c = _io_smemdata->buf[_io_smemdata->pos++];
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

int stputc(int c, STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return fputc(c, stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      if (_io_smemdata->end >= _io_smemdata->size) {
        _io_smemdata->size += 1024;
        _io_smemdata->buf = realloc(_io_smemdata->buf, _io_smemdata->size);
        if (!_io_smemdata->buf) {
          _io_smemdata->size = 0;
          _io_smemdata->end = 0;
          _io_smemdata->pos = 0;
          return EOF;
        }
      }
  
      if (_io_smemdata->pos < _io_smemdata->end)
        memmove(
          _io_smemdata->buf + _io_smemdata->pos + 1,
          _io_smemdata->buf + _io_smemdata->pos,
          _io_smemdata->end - _io_smemdata->pos);
  
      _io_smemdata->end++;
      return _io_smemdata->buf[_io_smemdata->pos++] = c;
    }
  }
  return EOF;
}

int stputs(const char *s, STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return fputs(s, stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      for (; *s; s++)
        if (stputc(*s, stream) == EOF)
          return EOF;
      return 0;
    }
  }
  return EOF;
}

int stseek(STREAM *stream, int offset, int whence)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return fseek(stream->data, offset, whence);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY) {
      switch (whence) {
        case SEEK_SET:
          _io_smemdata->pos = offset;
          break;
        case SEEK_CUR:
          _io_smemdata->pos += offset;
          break;
        case SEEK_END:
          _io_smemdata->pos = _io_smemdata->end + offset;
          break;
        default:
          return -1;
      }
      if (_io_smemdata->pos < 0)
        _io_smemdata->pos = 0;
      else if (_io_smemdata->pos > _io_smemdata->end)
        _io_smemdata->pos = _io_smemdata->end;
      return 0;
    }
  }
  return -1;
}

int sttell(STREAM *stream)
{
  if (stream) {
    if (stream->type == _IO_STREAM_TYPE_FILE)
      return ftell(stream->data);
    else if (stream->type == _IO_STREAM_TYPE_MEMORY)
      return _io_smemdata->pos;
  }
  return -1;
}

