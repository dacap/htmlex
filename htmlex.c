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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "htmlex.h"
#include "macros.h"
#include "stream.h"
#include "tags.h"

/* arguments for the input file */
char *args[MAX_ARGS];
int nargs = 0;

/* verbose mode stuff */
int verbose_level = 0;

/* the active output stream */
struct _IO_STREAM *_o_stream = NULL;

/* last path utiliced by the `try_fopen' function */
char success_path[256];

/* `if' states */
char ifs[MAX_IFS];
int nifs = 0;

/* we can can attach text in the output */
int can_attach;

/* name of the executable */
static char *name = NULL;

/* is the program calculating dependencies? */
static int calculating_deps = FALSE;
static STREAM *depstream = NULL;

/* paths where we can find/create files */
#define MAX_PATHS 256
static char *paths[MAX_PATHS];
static int npaths = 0;

/* tries open a file in all posible locations */
struct _IO_STREAM *try_sopen(const char *filename, const char *mode)
{
  STREAM *stream;
  strcpy(success_path, filename);
  stream = stopen(success_path, mode);
  if (!stream) {
    int c;
    for (c=0; c<npaths; c++) {
      sprintf(success_path, "%s/%s", paths[c], filename);
      stream = stopen(success_path, mode);
      if (stream)
        break;
    }
  }
  return stream;
}

/* gets the size of a file */
int get_filesize(const char *filename)
{
  FILE *f;
  int size = 0;
  f = fopen(filename, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
  }
  return size;
}

char *temp_filename(void)
{
  return strdup("_tmpfile.out");
}

/* gets the file name of the path */
static const char *get_filename(const char *path)
{
  char *s = strrchr(path, '/');
  return (s)? s+1: path;
}

/* replaces the file extension of the `path' with the new one
   indicated by `extension' */
static char *replace_extension(char *dest, const char *path, const char *extension)
{
  char *s = strrchr(get_filename(strcpy(dest, path)), '.');
  if (s)
    *s = 0;
  return strcat(dest, extension);
}

/* gets tokens from the string, the tokens can be separates by spaces
   (or tabs or new lines), and each one can enclosed by quotes or angular
   brackets, example:
   `hi, "good bye" <hi bye>' -> tok=hi,
                                tok=good bye
                                tok=<hi bye> */
static char *own_strtok(char *s)
{
  static char *d = NULL;
  char *r;

  if (s)
    d = s;
  else if (!d)
    return NULL;

  if ((*d != '\"') && (*d != '\'')) {
    for (r=d; !IS_BLANK(*d); d++) {
      if (*d == '<') {
        int c;
        for (c=0; ; d++) {
          if (*d == '<')
            c++;
          else if (*d == '>') {
            c--;
            if (c == 0) {
              break;
            }
          }
        }
      }
      else if (*d == 0) {
        d = NULL;
        break;
      }
    }
  }
  else {
    int quote = *d;
    r = ++d;
    while (*d != quote) {
      if (*d == 0) {
        d = NULL;
        break;
      }
      d++;
    }
  }

  if (d) {
    *d = 0;

    do {
      d++;
    } while ((*d) && IS_BLANK(*d));
    if (!*d)
      d = NULL;
  }

  return r;
}

/* process the file `in' and output the result to `out' file */
void process_file(struct _IO_STREAM *in, struct _IO_STREAM *out)
{
  struct _IO_STREAM *old_o_stream = _o_stream;
  char *s, buf[1024];

  _o_stream = out;

  new_ifs(IF_SPACE);
  update_ifs();

  while (stgets(buf, sizeof(buf), in)) {
    for (s=buf; *s; s++) {
      /* tag beginning */
      if ((*s == '<') && (s[1] == '!')) {
        char *tag = s+1;
        int c, i, used = FALSE;

        /* jump the comment? */
        if ((s[2] == '-') && (s[3] == '-')) {
          s += 2;
          for (;;) {
            if (strncmp(s, "-->", 3) == 0) {
              s += 2;
              break;
            }
            else if (*s == 0) {
              if (!stgets(buf, sizeof(buf), in))
                break;
              s = buf;
            }
            else {
              s++;
            }
          }
          continue;
        }

        /* jump nested tags */
        for (c=0; ; s++) {
          if (*s == '<')
            c++;
          else if (*s == '>') {
            c--;
            if (c == 0)
              break;
          }
          else if (*s == 0) {
            if (!stgets(buf+strlen(buf), sizeof(buf)-strlen(buf), in))
              break;
            s--;
          }
        }

        c = *s;
        *s = 0;

        PRINTF(2, "tag found: \"%s\"\n", tag+1);

        /* check for <!arg...> */
        if (strncmp(tag+1, "arg", 3) == 0) {
          if (can_attach) {
            /* <!args...> */
            if (tag[4] == 's') {
              char temp[32];
              sprintf(temp, "%d", nargs);
              stputs(temp, out);
            }
            /* <!arg[1-9][0-9]*...> */
            else {
              int arg = strtol(tag+4, NULL, 10);
              if ((arg > 0) && (arg <= nargs) && (args[arg-1]))
                stputs(args[arg-1], out);
            }
          }
          used = TRUE;
        }
        /* check for <!...> */
        if (!used) {
          for (i=0; i<ntags; i++)
            if (strncmp(tag+1, tags[i].name, strlen(tags[i].name)) == 0) {
              int x = tag[1+strlen(tags[i].name)];
              if (IS_BLANK(x) || (!x)) {
                char *tok, *toreplace;
                char *argv[MAX_ARGS];
                int argc = 0;

                for (tok=own_strtok(tag+2),
                     tok=own_strtok(NULL);
                     tok;
                     tok=own_strtok(NULL))
                  argv[argc++] = tok;

                if ((tags[i].if_tag) || (can_attach)) {
                  toreplace = tags[i].proc(argc, argv);
                  if (toreplace) {
                    stputs(toreplace, out);
                    free(toreplace);
                  }
                  PRINTF(2, "tag was processed\n");
                }
                else
                  PRINTF(2, "tag wasn't processed\n");

                used = TRUE;
                break;
              }
            }
        }
        /* well, this is an unknown tag */
        if (!used) {
          char *ptag = process_text(tag);
          if (can_attach) {
            stputc('<', out);
            stputs(ptag, out);
            stputc('>', out);
          }
          if (ptag) free(ptag);
        }

        if (!c) s--;
        else *s = c;
      }
      /* put a character in the output file */
      else if (can_attach) {
        int c = nmacros;

        /* this character is the start of a macro? */
        if (macros_table[(unsigned char)*s]) {
          /* search to see if really it is a macro */
          for (c =0; c<nmacros; c++)
            if (*s == *macros[c].name)
              if (strncmp(s, macros[c].name, strlen(macros[c].name)) == 0) {
                if (macros[c].value)
                  stputs(macros[c].value, out);
                s += strlen(macros[c].name) - 1;
                break;
              }
        }

        if (c == nmacros)
          stputc(*s, out);
      }
    }
  }

  delete_ifs();
  update_ifs();

  _o_stream = old_o_stream;
}

/* process a block of text like a file */
char *process_text(const char *s)
{
  STREAM *in, *out;
  char *r;
  in = stopen(NULL, NULL);
  out = stopen(NULL, NULL);
  stputs(s, in);
  stseek(in, 0, SEEK_SET);
  process_file(in, out);
  stclose(in);
  r = stbuffer(out);
  return (r)? r: strdup("");
}

void new_ifs(int type)
{
  int c;
  for (c=nifs; c>0; c--)
    ifs[c] = ifs[c-1];
  nifs++;
  ifs[0] = type;
}

void delete_ifs(void)
{
  int c;
  for (c=0; c<nifs; c++)
    ifs[c] = ifs[c+1];
  nifs--;
}

void update_ifs(void)
{
  int c;
  can_attach = TRUE;
  for (c=0; c<nifs; c++) {
    if (ifs[c] == IF_SPACE)
      break;
    if (ifs[c] != IF_INSIDE) {
      can_attach = FALSE;
      break;
    }
  }
}

void add_deps(const char *s)
{
  if (calculating_deps) {
    PRINTF(1, "new dependency: \"%s\"\n", s);

    if (!depstream)
      depstream = stopen(NULL, NULL);

    stputs(s, depstream);
    stputc('\n', depstream);
  }
}

void out_deps(void)
{
  if (calculating_deps) {
    char buf[256];
    int x;
    stseek(depstream, 0, SEEK_SET);
    /* file name */
    stgets(buf, sizeof(buf), depstream);
    buf[strlen(buf)-1] = 0;
    x = printf("%s:", buf);
    /* dependencies */
    while (stgets(buf, sizeof(buf), depstream)) {
      buf[strlen(buf)-1] = 0;

      x += strlen(buf);
      if (x > 76) {
        printf(" \\\n");
        x = strlen(buf);
      }
      else
        x += printf(" ");

      printf(buf);
    }
    printf("\n");
    stclose(depstream);
    depstream = NULL;
  }
}

void PRINTF(int level, const char *format, ...)
{
  if (level <= verbose_level) {
    char buf[1024];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    fprintf(stderr, "%s: %s", name, buf);
  }
}

/* shows the program usage information */
static void usage(void)
{
  printf("\
Usage:\n\
\n\
  htmlex [ options ] [ files... ]\n\
\n\
Options:\n\
\n\
  -c   compiles all subsequent files (use the active output file names\n\
       or defaults to converting from `file.htex' to `file.html')\n\
  -o   adds output files (note: must be used before the `-c')\n\
  -a   adds arguments for the input files (note: must be used before the `-c')\n\
  -i   adds all subsequent arguments to search include paths\n\
  -d   calculates dependencies of the input files (output to STDOUT)\n\
  -v   activates the verbose mode (to see what htmlex does)\n\
  -V   very verbose mode\n\
  -h   displays help screen and exit\n\
  --   terminates a -c, -o, -a or -i list\n\
\n\
Report bugs and patches to <dacap@users.sourceforge.net>\n\
");
}

/* main function */
int main(int argc, char *argv[])
{
  #define MAX_FILES 256
  char *files[MAX_FILES];
  int nfiles = 0;
  int process_stdin = TRUE;
  int compile_next = FALSE;
  int output_next = FALSE;
  int argument_next = FALSE;
  int include_next = FALSE;
  int c, i, j;

  atexit(remove_macros);

  name = argv[0];

  PRINTF(1, "processing arguments...\n");

  for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      compile_next = FALSE;
      output_next = FALSE;
      argument_next = FALSE;
      include_next = FALSE;

      for (j=1; argv[i][j]; j++) {
        switch (argv[i][j]) {
          case 'c': compile_next = TRUE; break;
          case 'o': output_next = TRUE; break;
          case 'a': argument_next = TRUE; break;
          case 'i': include_next = TRUE; break;
          case 'd': calculating_deps = TRUE; break;
          case 'v': verbose_level = 1; break;
          case 'V': verbose_level = 2; break;
          case 'h':
            usage();
            exit(0);
            break;
          case '-': break;
          default:
            PRINTF(0, "%s: unknow option\n", argv[i]);
            exit(1);
            break;
        }
      }
    }
    /* new output file */
    else if (output_next) {
      files[nfiles++] = argv[i];
      PRINTF(1, "new output file: \"%s\"\n", argv[i]);
    }
    /* new arguments for the input files */
    else if (argument_next) {
      args[nargs++] = argv[i];
      PRINTF(1, "new argument: \"%s\"\n", argv[i]);
    }
    /* new path for inclusion of files */
    else if (include_next) {
      paths[npaths++] = argv[i];
      PRINTF(1, "new path: \"%s\"\n", argv[i]);
    }
    else {
      STREAM *in, *out;
      char buf[256];

      PRINTF(1, "processing \"%s\" file\n", argv[i]);

      /* open the input file */
      in = try_sopen(argv[i], "r");
      if (!in) {
        PRINTF(0, "%s: file not found\n", argv[i]);
        exit(1);
      }

      /* output to file */
      if (compile_next) {
        /* we can get a file name from the output's files */
        if (nfiles > 0) {
          strcpy(buf, files[0]);

          nfiles--;
          for (c=0; c<nfiles; c++)
            files[c] = files[c+1];
        }
        /* auto generate the output file name */
        else {
          replace_extension(buf, success_path, ".html");
        }

        PRINTF(1, "output to \"%s\" file\n", buf);

        if (calculating_deps) {
          add_deps(buf);
          add_deps(argv[i]);
          out = NULL;
        }
        else {
          out = try_sopen(buf, "w");
          if (!out) {
            PRINTF(0, "%s: can't create file\n", buf);
            exit(1);
          }
        }
      }
      /* output to STDOUT */
      else {
        PRINTF(1, "output to STDOUT\n");

        add_deps(argv[i]);

        if (calculating_deps)
          out = NULL;
        else
          out = stfile(stdout);

        /* all next arguments for the file */
        nargs = 0;
        for (i++; i<argc; i++)
          args[nargs++] = argv[i];
      }

      /* process the file */
      process_file(in, out);

      /* close the files */
      stclose(in);
      stclose(out);

      PRINTF(1, "done with \"%s\" file\n", argv[i]);

      out_deps();

      process_stdin = FALSE;
    }
  }

  /* we must process STDIN because none file was process yet */
  if ((process_stdin) && (!calculating_deps)) {
    STREAM *in, *out;

    PRINTF(1, "processing STDIN\n");

    in = stfile(stdin);

    if (nfiles > 0) {
      out = try_sopen(files[0], "w");
      if (!out) {
        PRINTF(0, "%s: can't create file\n", files[0]);
        exit(1);
      }
    }
    else
      out = stfile(stdout);

    process_file(in, out);
    stclose(in);
    stclose(out);

    PRINTF(1, "done with STDIN\n");
  }

  PRINTF(1, "all done\n");

  return 0;
}



