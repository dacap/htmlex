#define VERSION_STRING "0.1"
#define BUG_ADDRESS "dacap@users.sourceforge.net"
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
#include <ctype.h>

#include "tag.h"
#include "bool.h"
#include "expr.h"
#include "macro.h"
#include "memory.h"
#include "string.h"
#include "stream.h"



/* estados del <if> */
#define IF_STATUS_NULL     0 /* fuera de un <if> */
#define IF_STATUS_TRUE     1 /* el anterior <if> fue verdadero */
#define IF_STATUS_FALSE    2 /* el anterior <if> fue falso */
#define IF_STATUS_WAIT     3 /* se est  esperando el cierre </if> */

#define IF_MAX            64 /* niveles de anidamientos posibles de <if> */



/* nombre del programa */
static char *name = NULL;

/* indica si se quieren mostrar todo tipo de mensajes (en stderr) para
   ver que es lo que _realmente_ sucede internamente en el programa */
static int verbose = FALSE;

/* indica si hay que imprimir solamente las dependencias del fichero
   de entrada */
static int dependencies = FALSE;

/* estados del <if> */
static int if_status[IF_MAX] = { IF_STATUS_NULL };

/* contador de niveles de <if> */
static int if_count = 0;

/* lista de macros */
static MACRO *macro_list = NULL;

/* indica si est  permitido procesar etiquetas */
static int tags_allowed = TRUE;

/* indica si est  permitido procesar macros */
static int macros_allowed = TRUE;

/* flujo de memoria para guardar las rutas de acceso disponibles para
   abrir archivos */
static STREAM *include_stream = NULL;

/* £ltima ruta utilizada para abrir el fichero (ver `include_fopen') */
static char include_path[256];



/* funciones en este fichero (por orden de aparici¢n) */
static FILE *include_fopen(const char *filename, const char *mode);

static STREAM *read_line(STREAM *src, STREAM *dest);

static char *get_property_value(TAG *tag, const char *name);

static int print_allowed();

static int process_tag(STREAM *src, STREAM *dest, const char *s, int argc, char **argv);
static void process_string(STREAM *src, STREAM *dest, const char *s, int argc, char **argv);
static void process_char(STREAM *src, STREAM *dest, STREAM *stream, int argc, char **argv);
static void process_file(STREAM *src, STREAM *dest, int argc, char **argv);



/* intenta abrir un fichero en todas las ubicaciones posibles (indicadas
   por las rutas de acceso en `include_stream') */
static FILE *include_fopen(const char *filename, const char *mode)
{
  char buf[256];
  FILE *file;

  /* el primer direcetorio a probar es el actual */
  strcpy_ex(include_path, ".");

  /* intentar abrir el fichero desde la ubicaci¢n actual */
  file = fopen(filename, mode);
  if (!file) {
    /* intentar abrir el archivo desde todas las ubicaciones posibles */

    /* leer l¡nea por l¡nea (cada l¡nea de `include_stream' contiene
       una ruta de acceso) desde el principio */
    stream_seek(include_stream, 0, STREAM_SEEK_SET);
    while (stream_gets(include_stream, include_path, sizeof(include_path))) {
      /* construir la ruta de acceso m s el nombre del fichero */
      sprintf(buf, "%s/%s", chop_ex(include_path), filename);

      /* intentar abrir */
      file = fopen(buf, mode);

      /* si se logr¢ encontrar el fichero, terminar el proceso */
      if (file)
        break;
    }
  }

  return file;
}



/* lee una l¡nea del flujo origen (src) y la inserta en el flujo
   destino (dest), si la l¡nea es mayor a 256 caracteres, la misma
   ser  leida en bloques de 256 caracteres y se ir  insertando cada
   uno por vez en el flujo destino hasta llegar al final, en otras
   palabras: "lee del flujo origen una l¡nea *COMPLETA* y la inserta
   en el flujo destino"; devuelve `dest' si es que se pudo leer una
   l¡nea, si no NULL */
static STREAM *read_line(STREAM *src, STREAM *dest)
{
  /* flujo a retornar (por ahora NULL) */
  STREAM *stream = NULL;
  char buf[256];
  int pos;

  /* guardar la posici¢n actual en el flujo destino */
  pos = stream_tell(dest);

  /* leer una l¡nea de a bloques de 256 caracteres */
  while (stream_gets(src, buf, sizeof(buf))) {
    /* el flujo a devolver ser  `dest' ya que se puedo leer una l¡nea */
    stream = dest;

    /* insertar la l¡nea en el flujo destino */
    stream_puts(dest, buf);

    /* si es que ya se pudo leer toda la l¡nea completa, frenar el proceso */
    if (buf[strlen_ex(buf)-1] == '\n')
      break;
  }

  /* restaurar la posici¢n en el flujo destino */
  stream_seek(dest, pos, STREAM_SEEK_SET);

  /* devolver el flujo */
  return stream;
}



/* obtiene el valor de la propiedad con el nombre indicado */
static char *get_property_value(TAG *tag, const char *name)
{
  PROPERTY *prop = get_property(tag, name);
  return (prop)? prop->value: NULL;
}



/* devuelve TRUE si se permite imprimir texto (todo dependiendo
   de los estados del <if>) */
static int print_allowed()
{
  int c;

  for (c=if_count; (if_status[c] != IF_STATUS_NULL); c--)
    if ((if_status[c] == IF_STATUS_FALSE) ||
        (if_status[c] == IF_STATUS_WAIT))
      return FALSE;

  return TRUE;
}



/* procesa una etiqueta, retorna TRUE si es que la etiqueta era
   realmente HTMLex o FALSE si era simplemente HTML.
   - transforma la etiqueta que est  representaba por una cadena
     de caracteres (`s') a la estructura TAG (ver `tag_from_string');
   - procesa la etiqueta apartir de la estructura TAG obtenida;
     - ver el nombre que tiene;
     - si no es HTML salir de la funci¢n;
     - si es HTMLex procesarla;
   - borrar la estructura TAG;
*/
static int process_tag(STREAM *src, STREAM *dest, const char *s, int argc, char **argv)
{
#define TAG_NULL      0 /* <...> */
#define TAG_ARGS      1 /* <args> */
#define TAG_ARGV      2 /* <argN> */
#define TAG_INCLUDE   3 /* <include ...> */
#define TAG_IF        4 /* <if ...> */
#define TAG_ELIF      5 /* <elif ...> */
#define TAG_ELSE      6 /* <else ...> */
#define TAG_ENDIF     7 /* </if> */
#define TAG_MACRO     8 /* <macro ...> */

  STREAM *stream;
  PROPERTY *prop;
  char buf[256];
  int i, whatag;
  TAG *tag;

  /* por ahora es una etiqueta desconocida */
  whatag = TAG_NULL;

  /* convertir la cadena que representa una etiqueta en la estructura TAG */
  tag = tag_from_string(s);

  /* pasar el primer caracter del nombre de la etiqueta a min£scula */
  tag->name[0] = tolower(tag->name[0]);

  /* averiguar qu‚ etiqueta es dependiendo del primer caracter de su nombre */
  switch (tag->name[0]) {

    case 'a':
      if (stricmp_ex(tag->name, "args") == 0)
        whatag = TAG_ARGS;
      else if (strni2cmp_ex(tag->name, "arg") == 0)
        whatag = TAG_ARGV;
      break;

    case 'e':
      if (stricmp_ex(tag->name, "elif") == 0) {
        whatag = TAG_ELIF;
        prop = get_property(tag, "");
        if (prop)
          prop->flags |= PROPERTY_FLAG_TAGS
                      |  PROPERTY_FLAG_MACROS
                      |  PROPERTY_FLAG_EXPR;
      }
      else if (stricmp_ex(tag->name, "else") == 0)
        whatag = TAG_ELSE;
      break;

    case 'i':
      if (stricmp_ex(tag->name, "include") == 0) {
        whatag = TAG_INCLUDE;
        for (i=1, prop=get_property(tag, "file"); prop;
             i++, prop=get_property(tag, buf)) {
          prop->flags |= PROPERTY_FLAG_TAGS
                      |  PROPERTY_FLAG_MACROS;
          sprintf(buf, "arg%d", i);
        }
      }
      else if (stricmp_ex(tag->name, "if") == 0) {
        whatag = TAG_IF;
        prop = get_property(tag, "");
        if (prop)
          prop->flags |= PROPERTY_FLAG_TAGS
                      |  PROPERTY_FLAG_MACROS
                      |  PROPERTY_FLAG_EXPR;
      }
      break;

    case 'm':
      if (stricmp_ex(tag->name, "macro") == 0) {
        whatag = TAG_MACRO;
        prop = get_property(tag, "value");
        if (prop)
          prop->flags |= PROPERTY_FLAG_TAGS
                      |  PROPERTY_FLAG_MACROS
                      |  PROPERTY_FLAG_EXPR;
      }
      break;

    case '/':
      if (stricmp_ex(tag->name, "/if") == 0)
        whatag = TAG_ENDIF;
      break;
  }

  /* esta etiqueta es HTML normal */
  if (whatag == TAG_NULL) {
    /* borrar la estructura */
    destroy_tag(tag);

    /* si se est  dentro de un <if> y la £ltima comparaci¢n no di¢
       verdadero, hay que saltear todo el texto hasta el cierre </if> */
    if ((if_status[if_count] != IF_STATUS_NULL) &&
        (if_status[if_count] != IF_STATUS_TRUE))
      /* saltearse la etiqueta */
      return TRUE;
    else
      /* la etiqueta no se utiliz¢, hay que imprimirla */
      return FALSE;
  }

  /* procesar cada una de las propiedades para ver si tienen
     macros y/o etiquetas en sus respectivos valores */
  for (i=0; i<tag->nprop; i++) {
    /* si no la propiedad no tiene ninguna bandera en especial,
       su valor queda intacto */
    if (!tag->prop[i].flags)
      continue;

    /* se permiten procesar etiquetas? */
    if (tag->prop[i].flags & PROPERTY_FLAG_TAGS)
      tags_allowed = TRUE;
    else
      tags_allowed = FALSE;

    /* se permiten procesar macros? */
    if (tag->prop[i].flags & PROPERTY_FLAG_MACROS)
      macros_allowed = TRUE;
    else
      macros_allowed = FALSE;

    /* nuevo estado de <if> a NULL */
    if_status[++if_count] = IF_STATUS_NULL;

    /* crear un flujo temporario en memoria */
    stream = stream_open(NULL);

    /* procesar el valor actual de la propiedad y dejar los resultados
       en dicho flujo... */
    process_string(NULL, stream, tag->prop[i].value, argc, argv);

    /* colocar el caracter nulo al final del flujo */
    stream_putc(stream, 0);

    /* liberar la memoria ocupada por el valor anterior de la propiedad */
    free_ex(tag->prop[i].value);

    if (!(tag->prop[i].flags & PROPERTY_FLAG_EXPR)) {
      /* conseguir una copia fresca del texto en el flujo */
      tag->prop[i].value = strdup_ex(stream_buffer(stream));
    }
    else {
      /* conseguir el resultado de la expresi¢n */
      tag->prop[i].value = eval_expr(stream_buffer(stream));
    }

    /* borrar el flujo en memoria */
    stream_close(stream);

    /* volver al estado del <if> anterior */
    if_count--;
  }

  /* permitir procesar etiquetas nuevamente */
  if (!tags_allowed)
    tags_allowed = TRUE;

  /* permitir procesar macros nuevamente */
  if (!macros_allowed)
    macros_allowed = TRUE;

  /* imprimir la informaci¢n de la etiqueta a partir de su estructura */
  if (verbose) {
    fprintf(stderr, "tag struct: name=\"%s\"", tag->name);
    for (i=0; i<tag->nprop; i++)
      fprintf(stderr, ", %s=\"%s\"", tag->prop[i].name, tag->prop[i].value);
    fputc('\n', stderr);
  }

  /* hacer diferentes cosas dependiendo de que etiqueta es */
  switch (whatag) {

    case TAG_ARGS:
      sprintf(buf, "%d", argc);
      process_string(src, dest, buf, argc, argv);
      break;

    case TAG_ARGV:
      i = strtol(tag->name+3, NULL, 10);
      if ((i >= 0) && (i < argc))
        process_string(src, dest, argv[i], argc, argv);
      break;

    case TAG_INCLUDE:
      {
        char *filename;
        char **file_argv;
        int file_argc;
        FILE *file;

        /* obtener el nombre del fichero */
        filename = get_property_value(tag, "file");

        /* si se especifico el nombre */
        if ((filename) && (*filename)) {
          /* mensaje de referencia */
          if (verbose)
            fprintf(stderr, "include file: \"%s\"\n", filename);

          /* abrir el fichero */
          file = include_fopen(filename, "r");

          /* error */
          if (!file) {
            fprintf(stderr, "%s: %s: %s\n", name, filename, strerror(errno));
          }
          /* procesar el fichero */
          else {
            /* dependencias? */
            if (dependencies)
              fprintf(stdout, " \\\n\t%s/%s", include_path, filename);

            /* argumentos nulos */
            file_argc = 0;
            file_argv = NULL;

            /* obtener todos los argumentos */
            for (prop=get_property(tag, "file"); prop;
                 prop=get_property(tag, buf)) {
              file_argc++;
              file_argv = (char **)realloc_ex(file_argv, sizeof(char *) * file_argc);
              file_argv[file_argc - 1] = prop->value;
              sprintf(buf, "arg%d", file_argc);
            }

            /* crear un flujo relacionado con el fichero de entrada */
            stream = stream_open(file);

            /* procesar el fichero */
            process_file(stream, dest, file_argc, file_argv);

            /* borrar el flujo de caracteres */
            stream_close(stream);

            /* cerrar el fichero */
            fclose(file);

            /* liberar la memoria ocupada por lo argumentos */
            free_ex(file_argv);
          }
        }
      }
      break;

    case TAG_ELIF:
        /* si anteriores <if> NO dieron FALSE */
        if (if_status[if_count] != IF_STATUS_FALSE) {
          /* si el anterior <if> ya di¢ TRUE */
          if (if_status[if_count] == IF_STATUS_TRUE)
            /* hay que poner al estado del <if> para que espere el cierre */
            if_status[if_count] = IF_STATUS_WAIT;

          /* frenar el proceso de todos modos */
          break;
        }

        /* si anteriores <if> dieron FALSE, esta etiqueta debe ser
           procesada tal cual un <if> normal, para ver si su comparaci¢n
           puede llegar a dar TRUE... */

    case TAG_IF:
      {
        char *expr = get_property_value(tag, "");
        int res;

        /* obtener si la expresi¢n es verdadera o falsa
           (cadena nula=FALSE; si es una d¡gito=depende de cu l
            sea la cifra; si es una cadena cualquiera=TRUE) */
        res = ((!expr) || !(*expr))? FALSE:
              (isdigit_ex(expr))? (strtol(expr, NULL, 0)? TRUE: FALSE): TRUE;

        /* nuevo estado <if> */
        if (whatag == TAG_IF)
          if_count++;

        /* si la expresi¢n es verdadera, colocamos IF_STATUS_TRUE para
           poder imprimir todo hasta llegar a un <elif>, <else> o </if>,
           si la expresi¢n es falsa, colocamos IF_STATUS_FALSE para
           poder saltearnos todo hasta llegar a un <elif>, <else> o </if>. */
        if_status[if_count] = (res)? IF_STATUS_TRUE: IF_STATUS_FALSE;
      }
      break;

    case TAG_ELSE:
      /* si anteriores <if> fueron FALSEs */
      if (if_status[if_count] == IF_STATUS_FALSE) {
        /* el <else> es como un <if> asegurado (siempre TRUE) */
        if_status[if_count] = IF_STATUS_TRUE;
      }
      /* si es que alg£n anterior <if> ya di¢ TRUE */
      else if (if_status[if_count] == IF_STATUS_TRUE) {
        /* poner a `if_status' para que espere el cierre </if> */
        if_status[if_count] = IF_STATUS_WAIT;
      }
      break;

    case TAG_ENDIF:
      /* volver al anterior estado <if> */
      if_count--;
      break;

    case TAG_MACRO:
      {
        char *name = get_property_value(tag, "name");
        char *value = get_property_value(tag, "value");

        if ((name) && (name[0])) {
          /* agregar una macro */
          if (value) {
            MACRO *macro;

            /* si ya hay una macro con el mismo nombre, modificarla */
            macro = get_macro(&macro_list, name, -1);
            if (macro) {
              if (verbose)
                fprintf(stderr, "set macro: \"%s\" -> \"%s\"\n", name, value);

              /* borrar el valor anterior */
              if (macro->value)
                free_ex(macro->value);

              /* asignarle el nuevo valor */
              macro->value = strdup_ex(value);
            }
            /* si no hay una macro con este nombre, agregarla... */
            else {
              /* ...al menos que se trate de una macro con nombre ilegal
                 como las que comienzan con '<' o '>') */
              if ((name[0] == '<') || (name[0] == '>')) {
                if (verbose)
                  fprintf(stderr, "ilegal macro name: \"%s\"\n", name);
              }
              else {
                if (verbose)
                  fprintf(stderr, "new macro: \"%s\" -> \"%s\"\n", name, value);

                macro = create_macro();

                macro->name = strdup_ex(name);
                macro->value = strdup_ex(value);

                insert_macro(&macro_list, macro);
              }
            }
          }
          /* borrar la macro */
          else {
            if (verbose)
              fprintf(stderr, "delete macro: \"%s\"\n", name);

            destroy_macro(
              remove_macro(&macro_list,
                get_macro(&macro_list, name, -1)));
          }
        }
        /* borrar todas las macros */
        else if (get_property(tag, "reset")) {
          if (verbose)
            fprintf(stderr, "reset all macros\n");

          while (macro_list)
            destroy_macro(remove_macro(&macro_list, macro_list));
        }
      }
      break;
  }

  /* liberar la memoria ocupada por la etiqueta */
  destroy_tag(tag);

  /* la etiqueta era efectivamente HTMLex */
  return TRUE;
}



/* procesa una cadena de caracteres:
   - se crea un buffer con el contenido de la cadena `s';
   - se procesa dicho buffer caracter por caracter;
   - y luego se borra */
static void process_string(STREAM *src, STREAM *dest, const char *s, int argc, char **argv)
{
  STREAM *stream;

  stream = stream_open(NULL);
  stream_puts(stream, s);
  stream_seek(stream, 0, STREAM_SEEK_SET);

  while (!stream_eof(stream))
    process_char(src, dest, stream, argc, argv);

  stream_close(stream);
}



/* procesa un caracter, tambi‚n interpreta si el mismo es el comienzo
   tanto de una etiqueta como de una macro.
   src    = flujo origen, de donde sacar m s caracteres si es que `stream'
            necesita m s (como para la continuaci¢n de etiquetas).
   dest   = flujo de destino, donde se coloca el resultado de la operaci¢n.
   stream = flujo a procesar, donde se sacan los caracteres.
*/
static void process_char(STREAM *src, STREAM *dest, STREAM *stream, int argc, char **argv)
{
  char *s;
  int c;

  /* leer un caracter del flujo */
  c = stream_getc(stream);

  /* ver si el caracter es el inicio de una etiqueta */
  if ((tags_allowed) && (c == '<')) {
    STREAM *tag;
    int brackets;
    int prev;

    /* leer el caracter siguiente de la etiqueta */
    prev = c;
    c = stream_getc(stream);

    /* ver si se trata de la apertura de un comentario */
    if ((c == '!') &&
        (strn2cmp_ex(stream_buffer(stream) + stream_tell(stream), "--") == 0)) {
      char buf[4] = { 0, 0, 0, 0 };

      /* leer todo hasta llegar al final del comentario o
         hasta llegar al final del fichero */
      for ( ; (strn2cmp_ex(buf, "-->") != 0); ) {
        c = stream_getc(stream);

        if (c == EOF) {
          if (!read_line(src, stream))
            break;
        }
        else {
          memmove_ex(buf, buf+1, 3);
          buf[2] = c;
        }
      }
    }
    else {
      /* crear un flujo temporal para alojar la informaci¢n de la etiqueta */
      tag = stream_open(NULL);

      /* leer la etiqueta completa hasta su cierre (indicado por `>')
         o hasta llegar al final del fichero `src' */
      for (brackets=1; ; ) {
        /* se se encontr¢ el final de l¡nea */
        if (c == EOF) {
          /* leer otra l¡nea */
          if (!read_line(src, stream))
            break;
          else {
            /* leer otro caracter */
            prev = c;
            c = stream_getc(stream);
            continue;
          }
        }
        /* comienzo de una etiqueta dentro de esta */
        else if (c == '<') {
          brackets++;
        }
        /* final de una etiqueta */
        else if (c == '>') {
          brackets--;
          if (!brackets)
            break;
        }
        /* si el caracter anterior era el comienzo de un valor de
           una propiedad (`='), y el actual es comillas simples (')
           o comillas dobles (") */
        else if ((prev == '=') && ((c == '\'') || (c == '\"'))) {
          int c2 = c;

          stream_putc(tag, c);

          /* leer todo hasta el final de la comilla o hasta el final
             del fichero `src' */
          for (;;) {
            prev = c;
            c = stream_getc(stream);

            if (c == EOF) {
              if (!read_line(src, stream))
                break;
            }
            else if ((c == c2) && (prev != '\\'))
              break;
            else
              stream_putc(tag, c);
          }
        }

        /* imprimir el caracter */
        stream_putc(tag, c);

        /* leer otro caracter */
        prev = c;
        c = stream_getc(stream);
      }

      /* agregar un £ltimo caracter nulo al flujo de la etiqueta (para que
         el buffer tenga un final al utilizarlo como cadena de caracteres) */
      stream_putc(tag, 0);

      /* obtener el buffer del flujo */
      s = stream_buffer(tag);

      /* texto de muestra */
      if (verbose)
        fprintf(stderr, "tag string: \"%s\"\n", s);

      /* procesar la etiqueta ("tag prop prop2" sin los '<' y '>') */
      if (!process_tag(src, dest, s, argc, argv)) {
        /* si se trataba en realidad de una etiqueta HTML normal... */

        /* imprimir el inicio de la etiqueta */
        stream_putc(dest, '<');

        /* ir al inicio del flujo `tag' para imprimir la etiqueta HTML */
        stream_seek(tag, 0, STREAM_SEEK_SET);

        /* imprimir todo el contenido de la etiqueta HTML */
        while (!stream_eof(tag))
          process_char(src, dest, tag, argc, argv);

        /* imprimir el fin de la etiqueta */
        stream_putc(dest, '>');
      }

      /* borrar el flujo temporal que alojaba la etiqueta */
      stream_close(tag);
    }
  }
  else if (c != 0) {
    /* est  permitido imprimir caracteres? */
    if (!print_allowed())
      return;

    /* si est  permitido procesar macros */
    if (macros_allowed) {
      MACRO *iter, *iter2;

      s = NULL;

      /* ver si el caracter es el inicio de alguna macro */
      for (iter=macro_list; iter; iter=iter->next) {
        /* si la macro no est  utilizada a£n, y si el primer caracter de la
           macro coincide con el caracter a imprimir... */
        if ((!iter->used) && (c == iter->name[0])) {
          /* si todav¡a no se hizo, obtener un puntero a la cadena de
             caracteres que est  seguido al caracter que se lley¢
             (incluido este, por eso el `-1') */
          if (!s)
            s = stream_buffer(stream) + stream_tell(stream) - 1;

          /* ...comparar si los caracteres que siguen son totalmente iguales */
          if (strn2cmp_ex(s, iter->name) == 0) {
            /* BINGO!, a reemplazar se a dicho... */
            if (verbose)
              fprintf(stderr, "macro founded: \"%s\"\n", iter->name);

            /* saltearse los caracteres que ocupa el nombre de la macro
               (menos el primer caracter que ya fue leido) */
            stream_seek(stream, strlen_ex(iter->name) - 1, STREAM_SEEK_CUR);

            /* indicar que esta macro ya fue utilizada */
            iter->used = TRUE;

            /* reemplazar por el valor de la macro */
            process_string(src, dest, iter->value, argc, argv);

            /* buscar nuevamente por la macro en la lista (por las dudas ya que
               el valor de la macro (iter->value) pod¡a contener alguna etiqueta
               para borrarse a s¡ misma (como "<macro reset>" tambi‚n)) */
            for (iter2=macro_list; iter2; iter2=iter2->next) {
              /* si efectivamente sigue en la lista de macros... */
              if (iter2 == iter) {
                /* ...indicar que esta macro ya est  disponible para una
                   futura utilizaci¢n */
                iter->used = FALSE;
                break;
              }
            }

            /* listo */
            return;
          }
        }
      }
    }

    /* al fin y al cabo, solamente se trataba de un caracter com£n
       y corriente, imprimirlo en el flujo de salida y listo */
    stream_putc(dest, c);
  }
}



/* procesa un fichero (src: flujo de entrada, dest: flujo de salida):
   - crea el flujo principal para alojar las l¡neas;
   - lee l¡nea por l¡nea hasta que se llegue al final del fichero;
   - procesar el flujo caracter por caracter (ver `process_char');
   - se resetea el flujo para poder utilizarlo en la leida
     de otra nueva l¡nea;
   - se borra el flujo temporal;
*/
static void process_file(STREAM *src, STREAM *dest, int argc, char **argv)
{
  STREAM *stream;

  /* crear un flujo de caracteres para alojar una l¡nea */
  stream = stream_open(NULL);

  /* leer l¡nea por l¡nea */
  while (read_line(src, stream)) {
    /* procesar la l¡nea de texto */
    while (!stream_eof(stream))
      /* procesar un caracter */
      process_char(src, dest, stream, argc, argv);

    /* resetear el flujo para leer una nueva l¡nea desde cero */
    stream_close(stream);
    stream = stream_open(NULL);
  }

  /* borrar el flujo */
  stream_close(stream);
}



/* imprime un peque¤o mensaje de ayuda para el usuario y sale del programa
   con el c¢digo de salida `exit_code' (!= 0: error, muestra un m¡nimo
   cartel de c¢mo obtener la ayuda general) */
static void usage(int exit_code)
{
  if (exit_code != 0) {
    fprintf(stderr,
      "Try `%s --help' for more information.\n"
      , name);
  }
  else {
    fprintf(stdout,
      "Usage: %s [OPTION]... [[FILE [ARGS]] | [[--args [ARGS]...] < STDIN]]\n"
      "Compile the FILE (or STDIN) and output the results to STDOUT.\n"
      "\n"
      "  -a, --args      all next arguments are for the file\n"
      "  -I <dir>        add <dir> to the end of include paths\n"
      "  -M              generate make dependencies\n"
      "  -h, --header    output the HTTP header\n"
      "  -v, --verbose   explain what is being done\n"
      "      --help      display this help and exit\n"
      "      --version   output version information and exit\n"
      "\n"
      "Report bugs to <%s>.\n"
      , name, BUG_ADDRESS);
  }
  exit(exit_code);
}



/* funci¢n principal:
   1) procesa los argumentos;
   2) lee el fichero principal (puede ser stdin);
   3) procesa el fichero;
   4) cierra el fichero (si no es stdin);
   5) borra la memoria utilizada;
   6) sale del programa;
*/
int main(int argc, char *argv[])
{
  FILE *file = NULL;
  STREAM *src = NULL;
  STREAM *dest = NULL;
  char *filename = NULL;
  char **file_argv = NULL;
  int file_argc = 0;
  int args_memory = FALSE;
  int http_header = FALSE;
  int i, j;

  /* nombre del programa */
  name = *argv;

  /* leer los argumentos */
  for (i=1; i<argc; i++) {
    /* argv="-..." */
    if (argv[i][0] == '-') {
      /* argv="--..." */
      if (argv[i][1] == '-') {
        if (strcmp_ex(argv[i]+2, "args") == 0) {
          argv[i] = "stdin";
          file_argv = argv + i;
          file_argc = argc - i;
          break;
        }
        else if (strcmp_ex(argv[i]+2, "header") == 0) {
          http_header = TRUE;
        }
        else if (strcmp_ex(argv[i]+2, "verbose") == 0) {
          verbose = TRUE;
        }
        else if (strcmp_ex(argv[i]+2, "help") == 0) {
          usage(0);
        }
        else if (strcmp_ex(argv[i]+2, "version") == 0) {
          fprintf(stdout, "%s", VERSION_STRING);
          exit(1);
        }
        else {
          fprintf(stderr, "%s: unrecognized option `%s'\n", name, argv[i]);
          usage(1);
        }
      }
      /* argv="-..." */
      else {
        int included = FALSE;

        for (j=1; argv[i][j]; j++) {
          switch (argv[i][j]) {

            case 'a':
              argv[i] = "stdin";
              file_argv = argv + i;
              file_argc = argc - i;
              i = argc;
              goto for_break;

            case 'I':
              if (!include_stream)
                include_stream = stream_open(NULL);

              stream_puts(include_stream, argv[i+1]);
              stream_putc(include_stream, '\n');

              included = TRUE;
              break;

            case 'M':
              dependencies = TRUE;
              break;

            case 'h':
              http_header = TRUE;
              break;

            case 'v':
              verbose = TRUE;
              break;

            default:
              fprintf(stderr, "%s: invalid option -- %c\n", name, argv[i][j]);
              usage(1);
              break;
          }
        }

        for_break:;

        if (included)
          i++;
      }
    }
    else {
      filename = argv[i];
      file_argv = argv + i;
      file_argc = argc - i;
      break;
    }
  }

  /* utilizar el fichero correspondiente */
  if (filename) {
    file = include_fopen(filename, "r");
    if (!file) {
      fprintf(stderr, "%s: %s: %s\n", name, filename, strerror(errno));
      exit(1);
    }
  }
  else {
    file = stdin;
  }

  /* argumentos por defecto para el fichero HTMLex */
  if (!file_argc) {
    file_argc = 1;
    file_argv = (char **)malloc_ex(sizeof(char *) * file_argc);
    file_argv[0] = "stdin";

    args_memory = TRUE;
  }

  /* crear el flujo de caracteres relacionado con el fichero de entrada */
  src = stream_open(file);

  /* crear el flujo de caracteres relacionado con el fichero de salida */
  if (!dependencies)
    dest = stream_open(stdout);
  else
    fprintf(stdout, "%s:", filename);

  /* imprimir la cabecera HTTP (si as¡ se indic¢) */
  if (http_header)
    stream_puts(src, "Content-type: text/html\n\n");

  /* procesar el fichero */
  process_file(src, dest, file_argc, file_argv);

  /* texto para las dependencias */
  if (dependencies)
    fprintf(stdout, "\n\n");

  /* cerrar el flujo de caracteres de salida */
  stream_close(dest);

  /* cerrar el flujo de caracteres de entrada */
  stream_close(src);

  /* liberar la memoria ocupada por las rutas de acceso */
  stream_close(include_stream);

  /* cerrar el fichero */
  if (file != stdin)
    fclose(file);

  /* liberar la memoria ocupada por los argumentos */
  if (args_memory)
    free_ex(file_argv);

  /* liberar la memoria ocupada por las macros */
  while (macro_list)
    destroy_macro(remove_macro(&macro_list, macro_list));

  /* imprimir mensaje final de cuanta memoria fue utilizada */
  if (verbose)
    fprintf(stderr, "memory used: %d byte(s)\n", get_memory_used());

  /* salida normal */
  exit(0);

  /* exit() fall¢? */
  return 1;
}



