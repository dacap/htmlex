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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "htmlex.h"
#include "macros.h"
#include "stream.h"
#include "tags.h"

static char *tag_basename(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text(argv[0]);

    s = strrchr(filename, '/');
    if (!s)
      s = filename;

    s = strrchr(s, '.');
    if (s)
      *s = 0;

    s = strdup(filename);

    free(filename);
    return s;
  }
  else
    return NULL;
}

/* removes the last character */
static char *tag_chop(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s = process_text(argv[0]);
    if (*s)
      s[strlen(s)-1] = 0;
    return s;
  }
  else
    return NULL;
}

/* removes the leading and trailing whitespaces */
static char *tag_clean(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s = argv[0];
    /* clean leading whitespaces */
    while ((*s) && IS_BLANK(*s))
      s++;
    /* process the text */
    s = process_text(s);
    /* clean trailing whitespaces */
    while ((*s) && IS_BLANK(s[strlen(s)-1]))
      s[strlen(s)-1] = 0;
    return s;
  }
  else
    return NULL;
}

/* returns the directory */
static char *tag_dir(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text(argv[0]);

    s = strrchr(filename, '/');
    if (s) {
      s[1] = 0;
      s = filename;
    }
    else {
      s = "./";
    }
    s = strdup(s);

    free(filename);
    return s;
  }
  else
    return NULL;
}

/* executes an extern command and redirects the stdout */
static char *tag_exec(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = temp_filename();
    int outbak = dup(1);
    int outdsc = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
    char buf[4096] = "";
    STREAM *in;
    int c;
    /* make the command to run */
    for (c=0; c<argc; c++) {
      s = process_text(argv[c]);
      sprintf(buf+strlen(buf), "%s ", s);
      free(s);
    }
    /* redirect the stdout */
    fflush(stdout);
    dup2(outdsc, 1);
    /* run the command */
    system(buf);
    /* restore the stdout */
    dup2(outbak, 1);
    close(outdsc);
    close(outbak);
    /* read the redirected output from the file and put it
       to the active output stream */
    if (can_attach) {
      in = sopen(filename, "r");
      while (sgets(buf, sizeof(buf), in))
        sputs(buf, _o_stream);
      sclose(in);
    }
    /* close*/
    remove(filename);
    free(filename);
  }
  /* nothing to add */
  return NULL;
}

/* executes an extern command, redirects the stdout and process it */
static char *tag_exec_proc(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = temp_filename();
    int outbak = dup(1);
    int outdsc = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
    char buf[4096] = "";
    STREAM *in;
    int c;
    /* make the command to run */
    for (c=0; c<argc; c++) {
      s = process_text(argv[c]);
      sprintf(buf+strlen(buf), "%s ", s);
      free(s);
    }
    /* redirect the stdout */
    fflush(stdout);
    dup2(outdsc, 1);
    /* run the command */
    system(buf);
    /* restore the stdout */
    dup2(outbak, 1);
    close(outdsc);
    close(outbak);
    /* process the redirected output from the file and put it
       to the active output stream */
    if (can_attach) {
      in = sopen(filename, "r");
      process_file(in, _o_stream);
      sclose(in);
    }
    /* close*/
    remove(filename);
    free(filename);
  }
  /* nothing to add */
  return NULL;
}

/* returns in a human-readable format the file size */
static char *tag_file_size(int argc, char *argv[])
{
  if (argc >= 1) {
    char *filename = process_text(argv[0]);
    int size = get_filesize(filename);
    char format[32];
    if (size < 1024)
      sprintf(format, "%db", size);
    else if (size < 1024 * 1000)
      sprintf(format, "%0.1fk", (float)size / 1024.0);
    else
      sprintf(format, "%0.2fM", (float)size / 1024.0 / 1024.0);
    free(filename);
    return strdup(format);
  }
  else
    return NULL;
}

static char *tag_find(int argc, char *argv[])
{
  if (argc >= 1) {
    char *filename = process_text(argv[0]);
    STREAM *s = try_sopen(filename, "r");
    free(filename);
    if (s) {
      sclose(s);
      return strdup(success_path);
    }
  }
  return NULL;
}

static char *tag_include(int argc, char *argv[])
{
  /* if we can't put text, why we need to include the file? */
  if (!can_attach)
    return NULL;
  /* at least one argument: the file name */
  if (argc >= 1) {
    char *old_args[MAX_ARGS];
    char *new_args[MAX_ARGS];
    int old_nargs;
    int new_nargs;
    STREAM *in;
    char *s;
    int c;
    /* open the file */
    s = process_text(argv[0]);
    in = try_sopen(s, "r");
    free(s);
    /* warning, file not found */
    if (!in) {
      /* WARNING */
      return NULL;
    }
    /* add a dependency */
    add_deps(success_path);
    /* new arguments for the input file */
    new_nargs = argc-1;
    for (c=0; c<new_nargs; c++)
      new_args[c] = process_text(argv[c+1]);
    /* save the active arguments */
    old_nargs = nargs;
    for (c=0; c<old_nargs; c++)
      old_args[c] = args[c];
    /* put the new arguments to the active */
    nargs = new_nargs;
    for (c=0; c<nargs; c++)
      args[c] = new_args[c];
    /* process the file and close it, the output directly
       to the active file */
    process_file(in, _o_stream);
    sclose(in);
    /* restore the arguments by the old ones */
    nargs = old_nargs;
    for (c=0; c<nargs; c++)
      args[c] = old_args[c];
    /* delete the new arguments */
    for (c=0; c<new_nargs; c++)
      free(new_args[new_nargs]);
  }
  /* nothing to add */
  return NULL;
}

static int sort_macros(const void *e1, const void *e2)
{
  return strlen(((struct _macros *)e2)->name) -
         strlen(((struct _macros *)e1)->name);
}

static char *tag_macro(int argc, char *argv[])
{
  int c;

  if (!can_attach)
    return NULL;

  if (argc >= 1) {
    for (c=0; c<nmacros; c++)
      if (strcmp(argv[0], macros[c].name) == 0) {
        /* modify macro */
        if (argc >= 2) {
          char *value = process_text(argv[1]);
          if (macros[c].value) free(macros[c].value);
          macros[c].value = value;
        }
        /* remove macro */
        else {
          if (macros[c].name) free(macros[c].name);
          if (macros[c].value) free(macros[c].value);
          for (nmacros--; c<nmacros; c++)
            macros[c] = macros[c+1];
        }
        break;
      }

    /* new macro */
    if ((c == nmacros) && (argc >= 2)) {
      macros_table[(unsigned char)*argv[0]] = 1;

      macros[c].name = strdup(argv[0]);
      macros[c].value = process_text(argv[1]);
      nmacros++;

      qsort(macros, nmacros, sizeof(struct _macros), sort_macros);
    }
  }

  return NULL;
}

static char *tag_macro_reset(int argc, char *argv[])
{
  if (can_attach)
    remove_macros();

  return NULL;
}

static char *tag_notdir(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text(argv[0]);

    s = strrchr(filename, '/');
    s = (s)? s+1: filename;
    s = strdup(s);

    free(filename);
    return s;
  }
  else
    return NULL;
}

/* removes the first character */
static char *tag_shift(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s = process_text(argv[0]);
    int c;
    for (c=0; s[c]; c++)
      s[c] = s[c+1];
    return s;
  }
  else
    return NULL;
}

static char *tag_suffix(int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text(argv[0]);

    /* get file name position */
    s = strrchr(filename, '/');
    s = (s)? s+1: filename;

    /* get the last point */
    s = strrchr(s, '.');
    if (s) s = strdup(s);

    free(filename);
    return s;
  }
  else
    return NULL;
}

static char *_tocase(int argc, char *argv[], int (*convert)(int c))
{
  if (argc >= 1) {
    char *s = process_text(argv[0]);
    int beg, end, len = strlen(s);
    int c;

    if (argc >= 2) {
      beg = strtol(argv[1], NULL, 10);
      if (argc >= 3)
        end = strtol(argv[2], NULL, 10);
      else
        end = beg;
    }
    else {
      beg = 0;
      end = strlen(s)-1;
    }

    if (end < 0) end = len + end;

    beg = (beg < 0)? 0: (beg > len-1)? len-1: beg;
    end = (end < 0)? 0: (end > len-1)? len-1: end;

    for (c=beg; c<=end; c++)
      s[c] = (*convert)(s[c]);

    return s;
  }
  else
    return NULL;
}

static char *tag_tolower(int argc, char *argv[])
{
  return _tocase(argc, argv, tolower);
}

static char *tag_toupper(int argc, char *argv[])
{
  return _tocase(argc, argv, toupper);
}

static char *tag_version(int argc, char *argv[])
{
  return strdup(VERSION);
}

/* conditional blocks */

/* returns TRUE if `s' represents a number */
static int is_number(char *s)
{
  int r = TRUE;
  /* with sign */
  if ((*s == '+') || (*s == '-')) s++;
  /* octal or hexadecimal */
  if (*s == '0') {
    s++;
    /* hexadecimal */
    if (*s == 'x') {
      for (s++; *s; s++)
        if (((*s < '0') || (*s > '9')) &&
            ((*s < 'a') || (*s > 'f')) &&
            ((*s < 'A') || (*s > 'F'))) {
          r = FALSE;
          break;
        }
    }
    /* octal */
    else {
      for (; *s; s++)
        if ((*s < '0') || (*s > '7')) {
          r = FALSE;
          break;
        }
    }
  }
  /* decimal */
  else {
    for (; *s; s++)
      if ((*s < '0') || (*s > '9')) {
        r = FALSE;
        break;
      }
  }
  return r;
}

static int eval_if(int argc, char *argv[])
{
  int r = FALSE;
  /* one argument */
  if (argc == 1) {
    /* number */
    if (is_number(argv[0]))
      r = (strtol(argv[0], NULL, 0))? TRUE: FALSE;
    /* text */
    else
      r = (*argv[0])? TRUE: FALSE;
  }
  /* two arguments */
  else if (argc == 2) {
    if (strcmp(argv[0], "!") == 0) {
      /* number */
      if (is_number(argv[0]))
        r = (strtol(argv[0], NULL, 0))? FALSE: TRUE;
      /* text */
      else
        r = (*argv[1])? FALSE: TRUE;
    }
  }
  /* three arguments */
  else if (argc == 3) {
    /* number */
    if (is_number(argv[0]) && is_number(argv[2])) {
      int x, y;
      x = strtol(argv[0], NULL, 0);
      y = strtol(argv[2], NULL, 0);
      if (strcmp(argv[1], "==") == 0)
        r = (x == y);
      else if (strcmp(argv[1], "!=") == 0)
        r = (x != y);
      else if (strcmp(argv[1], "<") == 0)
        r = (x < y);
      else if (strcmp(argv[1], ">") == 0)
        r = (x > y);
      else if (strcmp(argv[1], "<=") == 0)
        r = (x < y);
      else if (strcmp(argv[1], ">=") == 0)
        r = (x > y);
    }
    /* text */
    else {
      if (strcmp(argv[1], "==") == 0)
        r = (strcmp(argv[0], argv[2]) == 0)? TRUE: FALSE;
      else if (strcmp(argv[1], "!=") == 0)
        r = (strcmp(argv[0], argv[2]) != 0)? TRUE: FALSE;
    }
  }
  /* return the result */
  return r;
}

static char *iftag_if(int argc, char *argv[])
{
  char *new_argv[MAX_ARGS];
  int c;
  /* process all arguments */
  for (c=0; c<argc; c++)
    new_argv[c] = process_text(argv[c]);
  /* evaluate the expression */
  if (eval_if(argc, new_argv))
    /* for a true expression: we are now inside the if-block */
    new_ifs(IF_INSIDE);
  else
    /* the evaluation give false: don't enter in this if-block */
    new_ifs(IF_NOTYET);
  /* update the if's states */
  update_ifs();
  /* free all arguments */
  for (c=0; c<argc; c++)
    free(new_argv[c]);
  /* nothing to add */
  return NULL;
}

static char *iftag_elif(int argc, char *argv[])
{
  char *new_argv[MAX_ARGS];
  int c;
  /* process all arguments */
  for (c=0; c<argc; c++)
    new_argv[c] = process_text(argv[c]);
  /* evaluate the expression */
  if (eval_if(argc, new_argv)) {
    /* for a true expression: if we not yet pass for any if-block */
    if (ifs[0] == IF_NOTYET)
      /* enter in this block */
      ifs[0] = IF_INSIDE;
  }
  else {
    /* the evaluation give false: if we are inside in previous if-block */
    if (ifs[0] == IF_INSIDE)
      /* now we need to jump all to the next `fi' (end of the if-block) */
      ifs[0] = IF_OUTSIDE;
  }
  /* update the if's states */
  update_ifs();
  /* free all arguments */
  for (c=0; c<argc; c++)
    free(new_argv[c]);
  /* nothing to add */
  return NULL;
}

static char *iftag_else(int argc, char *argv[])
{
  /* if we not yet pass for any if-block */
  if (ifs[0] != IF_OUTSIDE) {
    /* if all previous `if' and `elif' give false */
    if (ifs[0] == IF_NOTYET)
      /* now, is time to pass for the `else' */
      ifs[0] = IF_INSIDE;
    /* if some previous `if' or `elif' give true */
    else
      /* we need to jump the `else' block */
      ifs[0] = IF_OUTSIDE;
  }
  /* update the if's states */
  update_ifs();
  return NULL;
}

static char *iftag_fi(int argc, char *argv[])
{
  /* back to the old if's state */
  delete_ifs();
  /* update the if's states */
  update_ifs();
  return NULL;
}

/* lists of tags */

static struct _tags all_tags[] =
{
  /* standard tags */
  { "basename",       tag_basename        },
  { "chop",           tag_chop            },
  { "clean",          tag_clean           },
  { "dir",            tag_dir             },
  { "exec",           tag_exec            },
  { "exec-proc",      tag_exec_proc       },
  { "file-size",      tag_file_size       },
  { "find",           tag_find            },
  { "include",        tag_include         },
  { "macro",          tag_macro           },
  { "macro-reset",    tag_macro_reset     },
  { "notdir",         tag_notdir          },
  { "shift",          tag_shift           },
  { "suffix",         tag_suffix          },
  { "tolower",        tag_tolower         },
  { "toupper",        tag_toupper         },
  { "version",        tag_version         },
  /* special tags */
  { "if",             iftag_if            },
  { "elif",           iftag_elif          },
  { "else",           iftag_else          },
  { "fi",             iftag_fi            },
};

struct _tags *tags = all_tags;
int ntags = sizeof(all_tags) / sizeof(struct _tags);


