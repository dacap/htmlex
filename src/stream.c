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

#include "stream.h"
#include "string.h"
#include "memory.h"



typedef struct FILE_STREAM   /* flujo de ficheros */
{
  FILE *file;                /* controlador para las funciones stdio */
} FILE_STREAM;



typedef struct MEMORY_STREAM /* flujo de memoria */
{
  int max;                   /* l¡mite, si `size' lo pasa hay que reservar m s */
  int size;                  /* tama¤o actual */
  char *text;                /* texto */
  int pos;                   /* posici¢n actual */
} MEMORY_STREAM;



static FILE_STREAM *file_stream_alloc(FILE *file);
static void file_stream_free(FILE_STREAM *data);
static int file_stream_put(FILE_STREAM *data, int c);
static int file_stream_get(FILE_STREAM *data);
static int file_stream_seek(FILE_STREAM *data, int offset, int mode);
static int file_stream_tell(FILE_STREAM *data);
static int file_stream_eof(FILE_STREAM *data);

static MEMORY_STREAM *memory_stream_alloc(FILE *file);
static void memory_stream_free(MEMORY_STREAM *data);
static int memory_stream_put(MEMORY_STREAM *data, int c);
static int memory_stream_get(MEMORY_STREAM *data);
static int memory_stream_seek(MEMORY_STREAM *data, int offset, int mode);
static int memory_stream_tell(MEMORY_STREAM *data);
static int memory_stream_eof(MEMORY_STREAM *data);



/* crea (abre) un nuevo flujo de caracteres (si `file' es NULL,
   se crear  un flujo en memoria */
STREAM *stream_open(FILE *file)
{
  STREAM *stream;

  stream = (STREAM *)malloc_ex(sizeof(STREAM));
  if (!stream)
    return NULL;

  if (file) {
    stream->alloc = (void *)file_stream_alloc;
    stream->free  = (void *)file_stream_free;
    stream->put   = (void *)file_stream_put;
    stream->get   = (void *)file_stream_get;
    stream->seek  = (void *)file_stream_seek;
    stream->tell  = (void *)file_stream_tell;
    stream->eof   = (void *)file_stream_eof;
  }
  else {
    stream->alloc = (void *)memory_stream_alloc;
    stream->free  = (void *)memory_stream_free;
    stream->put   = (void *)memory_stream_put;
    stream->get   = (void *)memory_stream_get;
    stream->seek  = (void *)memory_stream_seek;
    stream->tell  = (void *)memory_stream_tell;
    stream->eof   = (void *)memory_stream_eof;
  }

  stream->data = stream->alloc(file);
  if (!stream->data) {
    free_ex(stream);
    return NULL;
  }

  return stream;
}



/* borra de la memoria (cierra) un flujo de caracteres */
void stream_close(STREAM *stream)
{
  if (stream) {
    if (stream->data)
      stream->free(stream->data);

    free_ex(stream);
  }
}



/* llama la funci¢n correspondiente para escribir un caracter en el flujo */
int stream_putc(STREAM *stream, int c)
{
  if ((stream) && (stream->put) && (c != EOF))
    return stream->put(stream->data, c);
  else
    return EOF;
}



/* llama la funci¢n correspondiente para obtener un caracter desde el flujo */
int stream_getc(STREAM *stream)
{
  if ((stream) && (stream->get))
    return stream->get(stream->data);
  else
    return EOF;
}



/* llama la funci¢n correspondiente para escribir una cadena de caracter
   completa en el flujo */
int stream_puts(STREAM *stream, const char *s)
{
  const char *s1 = s;

  if ((stream) && (stream->put) && (s)) {
    for (; *s; s++)
      if (stream->put(stream->data, *s) == EOF)
        return EOF;
  }

  return s - s1;
}



/* llama la funci¢n correspondiente para leer una l¡nea completa
   en el flujo */
char *stream_gets(STREAM *stream, char *s, int maxlength)
{
  int i, c;

  if ((stream) && (stream->get) && (s) && (maxlength > 0)) {
    for (i=0; i<maxlength-1; i++) {
      /* leer un caracter */
      c = stream->get(stream->data);

      /* si no es el fin del fichero */
      if (c != EOF) {
        /* si es una nueva l¡nea, ac  terminamos de leer */
        if (c == '\n') {
          s[i++] = c;
          break;
        }
        /* si no, asignamos el nuevo caracter y seguimos leyendo */
        else
          s[i] = c;
      }
      /* si es el fin del archivo salir */
      else
        break;
    }
    s[i] = 0;

    /* si no se pudo leer ni un solo caracter, devolver NULL,
       en caso contrario retornaremos el mismo `s' */
    return (!i)? NULL: s;
  }
  else
    return NULL;
}



/* llama la funci¢n correspondiente para ubicarse en un lugar especial
   dentro del flujo */
int stream_seek(STREAM *stream, int offset, int mode)
{
  if ((stream) && (stream->seek))
    return stream->seek(stream->data, offset, mode);
  else
    return -1;
}



/* llama la funci¢n correspondiente para devolver la ubicaci¢n
   actual dentro del flujo */
int stream_tell(STREAM *stream)
{
  if ((stream) && (stream->tell))
    return stream->tell(stream->data);
  else
    return -1;
}



/* llama la funci¢n correspondiente para devolver si la ubicaci¢n
   actual dentro del flujo es el fin del fichero (o fin-de-flujo
   mejor dicho) */
int stream_eof(STREAM *stream)
{
  if ((stream) && (stream->eof))
    return stream->eof(stream->data);
  else
    return -1;
}



/* obtiene el buffer de texto del flujo indicado, esta funci¢n solamente
   se aplica a flujos de caracteres que se alojen en memoria */
char *stream_buffer(STREAM *stream)
{
  if ((stream) && (stream->data) &&
      (((void *)stream->alloc) == ((void *)memory_stream_alloc)))
    return ((MEMORY_STREAM *)stream->data)->text;
  else
    return NULL;
}



static FILE_STREAM *file_stream_alloc(FILE *file)
{
  FILE_STREAM *data;

  data = (FILE_STREAM *)malloc_ex(sizeof(FILE_STREAM));
  if (!data)
    return NULL;

  data->file = file;
  return data;
}



static void file_stream_free(FILE_STREAM *data)
{
  if (data)
    free_ex(data);
}



static int file_stream_put(FILE_STREAM *data, int c)
{
  if (data)
    return fputc(c, data->file);
  else
    return EOF;
}



static int file_stream_get(FILE_STREAM *data)
{
  if (data)
    return fgetc(data->file);
  else
    return EOF;
}



static int file_stream_seek(FILE_STREAM *data, int offset, int mode)
{
  if (data)
    return fseek(data->file, offset,
      (mode == STREAM_SEEK_SET)? SEEK_SET:
      (mode == STREAM_SEEK_CUR)? SEEK_CUR:
      (mode == STREAM_SEEK_END)? SEEK_END: -1);
  else
    return -1;
}



static int file_stream_tell(FILE_STREAM *data)
{
  if (data)
    return ftell(data->file);
  else
    return -1;
}



static int file_stream_eof(FILE_STREAM *data)
{
  if (data)
    return feof(data->file);
  else
    return -1;
}



static MEMORY_STREAM *memory_stream_alloc(FILE *file)
{
  MEMORY_STREAM *data;

  if (file)
    return NULL;

  data = (MEMORY_STREAM *)malloc_ex(sizeof(MEMORY_STREAM));
  if (!data)
    return NULL;

  data->max = 0;
  data->size = 0;
  data->text = NULL;
  data->pos = 0;

  return data;
}



static void memory_stream_free(MEMORY_STREAM *data)
{
  if (data) {
    if (data->text)
      free_ex(data->text);

    free_ex(data);
  }
}



static int memory_stream_put(MEMORY_STREAM *data, int c)
{
  /* si el tama¤o supera el l¡mite */
  if (data->size >= data->max) {
    /* hay que aumentar el tama¤o del texto (de bloques de 256 caracteres) */
    data->max += 256;
    data->text = (char *)realloc_ex(data->text, sizeof(char) * data->max);
    if (!data->text)
      return EOF;
  }

  /* si la posici¢n no est  al fondo del fichero, quiere decir que
     est  en el medio, por lo que habr  que mover todos los caracteres
     de delante un espacio m s */
  if (data->pos < data->size) {
    memmove_ex(
      data->text+data->pos+1,
      data->text+data->pos,
      data->size - data->pos);
  }

  /* el flujo se hace un caracter m s grande */
  data->size++;

  /* escribir el caracter y avanzar un lugar */
  data->text[data->pos++] = c;

  /* retornar el caracter de entrada */
  return c;
}



static int memory_stream_get(MEMORY_STREAM *data)
{
  if (data->pos < data->size)
    return data->text[data->pos++];
  else
    return EOF;
}



static int memory_stream_seek(MEMORY_STREAM *data, int offset, int mode)
{
  if (data) {
    switch (mode) {
      case STREAM_SEEK_SET: data->pos = offset; break;
      case STREAM_SEEK_CUR: data->pos += offset; break;
      case STREAM_SEEK_END: data->pos = data->size + offset; break;
      default: return -1;
    }

    if (data->pos < 0)
      data->pos = 0;
    else if (data->pos > data->size)
      data->pos = data->size;

    return 0;
  }
  else
    return -1;
}



static int memory_stream_tell(MEMORY_STREAM *data)
{
  if (data)
    return data->pos;
  else
    return -1;
}



static int memory_stream_eof(MEMORY_STREAM *data)
{
  if (data)
    return (data->pos == data->size)? -1: 0;
  else
    return -1;
}



