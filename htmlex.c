/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001 by David A. Capello
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
#include "htmlex.h"
#include "macros.h"
#include "stream.h"
#include "tags.h"

/* arguments for the input file */
char *args[MAX_ARGS];
int nargs = 0;

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
  stream = sopen(success_path, mode);
  if (!stream) {
    int c;
    for (c=0; c<npaths; c++) {
      sprintf(success_path, "%s/%s", paths[c], filename);
      stream = sopen(success_path, mode);
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

  while (sgets(buf, sizeof(buf), in)) {
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
              if (!sgets(buf, sizeof(buf), in))
                break;
              s = buf;
            }
            else {
              s++;
            }
          }
          continue;
        }

        /* normal tag */
        for (c=0; ; s++) {
          if (*s == '<')
            c++;
          else if (*s == '>') {
            c--;
            if (c == 0)
              break;
          }
          else if (*s == 0) {
            if (!sgets(buf+strlen(buf), sizeof(buf)-strlen(buf), in))
              break;
            s--;
          }
        }

        c = *s;
        *s = 0;

        if (strncmp(tag+1, "arg", 3) == 0) {
          if (can_attach) {
            if (tag[4] == 's') {
              char temp[32];
              sprintf(temp, "%d", nargs);
              sputs(temp, out);
            }
            else {
              int arg = strtol(tag+4, NULL, 10);
              if ((arg > 0) && (arg <= nargs))
                sputs(args[arg-1], out);
            }
          }
          used = TRUE;
        }
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

                toreplace = tags[i].proc(argc, argv);
                if (toreplace) {
                  if (can_attach)
                    sputs(toreplace, out);
                  free(toreplace);
                }
                used = TRUE;
                break;
              }
            }
        }
        if (!used) {
          char *ptag = process_text(tag);
          if (can_attach) {
            sputc('<', out);
            sputs(ptag, out);
            sputc('>', out);
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
                  sputs(macros[c].value, out);
                s += strlen(macros[c].name) - 1;
                break;
              }
        }

        if (c == nmacros)
          sputc(*s, out);
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
  in = sopen(NULL, NULL);
  out = sopen(NULL, NULL);
  sputs(s, in);
  sseek(in, 0, SEEK_SET);
  process_file(in, out);
  sclose(in);
  r = sbuffer(out);
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
    if (!depstream)
      depstream = sopen(NULL, NULL);

    sputs(s, depstream);
    sputc('\n', depstream);
  }
}

void out_deps(void)
{
  if (calculating_deps) {
    char buf[256];
    int x;
    sseek(depstream, 0, SEEK_SET);
    /* file name */
    sgets(buf, sizeof(buf), depstream);
    buf[strlen(buf)-1] = 0;
    x = printf("%s:", buf);
    /* dependencies */
    while (sgets(buf, sizeof(buf), depstream)) {
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
    sclose(depstream);
    depstream = NULL;
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
  -c   compiles all next files (use the active output file names
       or generate they as `file.htex' to `file.html')\n\
  -o   adds output files (use it before the `-c')\n\
  -a   adds arguments for the input files (use it before the `-c')\n\
  -i   adds all next arguments to inclusion paths (for search files)\n\
  -d   calculates dependencies of the input files (output to STDOUT)\n\
  -h   shows this help and exit\n\
  -v   shows the htmlex version and exit\n\
  --   stops the -c, -o and -i arguments\n\
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
          case 'h':
            usage();
            exit(0);
            break;
          case 'v':
            printf("htmlex %s\n", VERSION);
            exit(0);
            break;
          case '-': break;
          default:
            fprintf(stderr, "%s: %s: unknow option\n", name, argv[i]);
            exit(1);
            break;
        }
      }
    }
    /* new output file */
    else if (output_next) {
      files[nfiles++] = argv[i];
    }
    /* new arguments for the input files */
    else if (argument_next) {
      args[nargs++] = argv[i];
    }
    /* new path for inclusion of files */
    else if (include_next) {
      paths[npaths++] = argv[i];
    }
    else {
      STREAM *in, *out;
      char buf[256];

      /* open the input file */
      in = try_sopen(argv[i], "r");
      if (!in) {
        fprintf(stderr, "%s: %s: file not found\n", name, argv[i]);
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

        if (calculating_deps) {
          add_deps(buf);
          add_deps(argv[i]);
          out = NULL;
        }
        else {
          out = try_sopen(buf, "w");
          if (!out) {
            fprintf(stderr, "%s: %s: can't create file\n", name, buf);
            exit(1);
          }
        }
      }
      /* output to STDOUT */
      else {
        add_deps(argv[i]);

        if (calculating_deps)
          out = NULL;
        else
          out = sfile(stdout);

        /* all next arguments for the file */
        nargs = 0;
        for (i++; i<argc; i++)
          args[nargs++] = argv[i];
      }

      /* process the file */
      process_file(in, out);

      /* close the files */
      sclose(in);
      sclose(out);

      out_deps();

      process_stdin = FALSE;
    }
  }

  /* we must process STDIN because none file was process yet */
  if ((process_stdin) && (!calculating_deps)) {
    STREAM *in, *out;

    in = sfile(stdin);

    if (nfiles > 0) {
      out = try_sopen(files[0], "w");
      if (!out) {
        fprintf(stderr, "%s: %s: can't create file\n", name, files[0]);
        exit(1);
      }
    }
    else
      out = sfile(stdout);

    process_file(in, out);
    sclose(in);
    sclose(out);
  }

  return 0;
}



