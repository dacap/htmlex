/* htmlex - a powerful hypertext markup language preprocessor
 * Copyright (C) 2001, 2002, 2003 by David A. Capello
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

static char *tag_basename (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text (argv[0]);

    s = strrchr (filename, '/');
    if (!s)
      s = filename;

    s = strrchr (s, '.');
    if (s)
      *s = 0;

    s = strdup (filename);

    free (filename);
    return s;
  }
  else
    return NULL;
}

/* removes the last character */
static char *tag_chop (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s = process_text (argv[0]);
    if (*s)
      s[strlen (s) - 1] = 0;
    return s;
  }
  else
    return NULL;
}

/* removes the leading and trailing whitespaces */
static char *tag_clean (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s = argv[0];
    /* clean leading whitespaces */
    while ((*s) && IS_BLANK (*s))
      s++;
    /* process the text */
    s = process_text (s);
    /* clean trailing whitespaces */
    while ((*s) && IS_BLANK (s[strlen (s) - 1]))
      s[strlen (s) - 1] = 0;
    return s;
  }
  else
    return NULL;
}

/* adds a dependency */
static char *tag_dep (int argc, char *argv[])
{
  /* at least one argument: the file name */
  if (argc >= 1) {
    char *filename = process_text (argv[0]);
    /* add a dependency */
    add_deps (filename);
    free (filename);
  }
  /* nothing to add */
  return NULL;
}

/* returns the directory */
static char *tag_dir (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text (argv[0]);

    s = strrchr (filename, '/');
    if (s) {
      s[1] = 0;
      s = filename;
    }
    else {
      s = "./";
    }
    s = strdup (s);

    free (filename);
    return s;
  }
  else
    return NULL;
}

/* executes an extern command and redirects the stdout */
static char *tag_exec (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = temp_filename ();
    int outbak = dup (1);
    int outdsc =
	open (filename, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    char buf[4096] = "";
    STREAM *in;
    int c;
    /* make the command to run */
    for (c = 0; c < argc; c++) {
      s = process_text (argv[c]);
      sprintf (buf + strlen (buf), "%s ", s);
      free (s);
    }
    /* redirect the stdout */
    fflush (stdout);
    dup2 (outdsc, 1);
    /* run the command */
    system (buf);
    /* restore the stdout */
    dup2 (outbak, 1);
    close (outdsc);
    close (outbak);
    /* read the redirected output from the file and put it
       to the active output stream */
    in = stopen (filename, "r");
    while (stgets (buf, sizeof (buf), in))
      stputs (buf, _o_stream);
    stclose (in);
    /* close */
    remove (filename);
    free (filename);
  }
  /* nothing to add */
  return NULL;
}

/* executes an extern command, redirects the stdout and process it */
static char *tag_exec_proc (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = temp_filename ();
    int outbak = dup (1);
    int outdsc =
	open (filename, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    char buf[4096] = "";
    STREAM *in;
    int c;
    /* make the command to run */
    for (c = 0; c < argc; c++) {
      s = process_text (argv[c]);
      sprintf (buf + strlen (buf), "%s ", s);
      free (s);
    }
    /* redirect the stdout */
    fflush (stdout);
    dup2 (outdsc, 1);
    /* run the command */
    system (buf);
    /* restore the stdout */
    dup2 (outbak, 1);
    close (outdsc);
    close (outbak);
    /* process the redirected output from the file and put it
       to the active output stream */
    in = stopen (filename, "r");
    process_file (in, _o_stream);
    stclose (in);
    /* close */
    remove (filename);
    free (filename);
  }
  /* nothing to add */
  return NULL;
}

/* returns in a human-readable format the file size */
static char *tag_file_size (int argc, char *argv[])
{
  if (argc >= 1) {
    char *filename = process_text (argv[0]);
    int size = get_filesize (filename);
    char format[256];
    /* add a dependency */
    if (size >= 0)
      add_deps (filename);
    /* make the file size in a human-readable format */
    if (size < 0)
      sprintf (format, "%d", size);
    else if (size < 1024)
      sprintf (format, "%d byte%c", size, size == 1 ? 0 : 's');
    else if (size < 1024 * 1024)
      sprintf (format, "%0.1f KB", (float)size / 1024.0);
    else if (size < 1024 * 1024 * 1024)
      sprintf (format, "%0.2f MB", (float)size / 1024.0 / 1024.0);
    else
      sprintf (format, "%0.2f GB", (float)size / 1024.0 / 1024.0 / 1024.0);
    free (filename);
    return strdup (format);
  }
  else
    return NULL;
}

static char *tag_find (int argc, char *argv[])
{
  if (argc >= 1) {
    char *filename = process_text (argv[0]);
    STREAM *s = try_sopen (filename, "r");
    free (filename);
    if (s) {
      stclose (s);
      return strdup (success_path);
    }
  }
  return NULL;
}

static char *tag_include (int argc, char *argv[])
{
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
    s = process_text (argv[0]);
    in = try_sopen (s, "r");
    free (s);
    /* warning, file not found */
    if (!in) {
      /* WARNING */
      return NULL;
    }
    /* add a dependency */
    add_deps (success_path);
    /* save the active arguments */
    old_nargs = nargs;
    for (c = 0; c < old_nargs; c++)
      old_args[c] = args[c];
    /* new arguments for the input file */
    new_nargs = argc - 1;
    for (c = 0; c < new_nargs; c++) {
      new_args[c] = process_text (argv[c + 1]);
      /* put the new arguments to the active */
      args[c] = new_args[c];
    }
    nargs = new_nargs;
    /* process the file and close it, the output directly
       to the active file */
    process_file (in, _o_stream);
    stclose (in);
    /* delete the new arguments */
    for (c = 0; c < new_nargs; c++)
      if (new_args[c])
	free (new_args[c]);
    /* restore the arguments by the old ones */
    nargs = old_nargs;
    for (c = 0; c < nargs; c++)
      args[c] = old_args[c];
  }
  /* nothing to add */
  return NULL;
}

static char *tag_macro (int argc, char *argv[])
{
  if (argc >= 1) {
    if (argc >= 2) {
      char *value = process_text (argv[1]);
      MACRO *macro = get_macro (macros_space[0], argv[0]);

      /* modify macro */
      if (macro) {
	modify_macro (macro, value);
      }
      /* new macro */
      else {
	macro = new_macro (NORMAL_MACRO, strdup (argv[0]), value);
	add_macro (macros_space[0], macro, TRUE);
      }
    }
    else {
      /* remove macro */
      remove_macro (macros_space[0], argv[0]);
    }
  }

  return NULL;
}

static char *tag_macro_reset (int argc, char *argv[])
{
  free_macro_list (macros_space[0]);
  macros_space[0] = new_macro_list ();
  return NULL;
}

static char *tag_notdir (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text (argv[0]);

    s = strrchr (filename, '/');
    s = (s) ? s + 1 : filename;
    s = strdup (s);

    free (filename);
    return s;
  }
  else
    return NULL;
}

/* removes the first character */
static char *tag_shift (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s = process_text (argv[0]);
    int c;
    for (c = 0; s[c]; c++)
      s[c] = s[c + 1];
    return s;
  }
  else
    return NULL;
}

static char *tag_suffix (int argc, char *argv[])
{
  if (argc >= 1) {
    char *s, *filename = process_text (argv[0]);

    /* get file name position */
    s = strrchr (filename, '/');
    s = (s) ? s + 1 : filename;

    /* get the last point */
    s = strrchr (s, '.');
    if (s)
      s = strdup (s);

    free (filename);
    return s;
  }
  else
    return NULL;
}

static char *_tocase (int argc, char *argv[], int (*convert) (int c))
{
  if (argc >= 1) {
    char *s = process_text (argv[0]);
    int beg, end, len = strlen (s);
    int c;

    if (argc >= 2) {
      beg = strtol (argv[1], NULL, 10);
      if (argc >= 3)
	end = strtol (argv[2], NULL, 10);
      else
	end = beg;
    }
    else {
      beg = 0;
      end = strlen (s) - 1;
    }

    if (end < 0)
      end = len + end;

    beg = (beg < 0) ? 0 : (beg > len - 1) ? len - 1 : beg;
    end = (end < 0) ? 0 : (end > len - 1) ? len - 1 : end;

    for (c = beg; c <= end; c++)
      s[c] = (*convert) (s[c]);

    return s;
  }
  else
    return NULL;
}

static char *tag_tolower (int argc, char *argv[])
{
  return _tocase (argc, argv, tolower);
}

static char *tag_toupper (int argc, char *argv[])
{
  return _tocase (argc, argv, toupper);
}

static char *tag_version (int argc, char *argv[])
{
  return strdup (VERSION);
}

static void read_function_body (STREAM * in, STREAM * out)
{
  char *s, *buf = current_line;
  int done = FALSE;

  do {
    for (s = current_col; *s; s++) {
      /* tag beginning */
      if ((*s == '<') && (s[1] == '!')) {
	char *tag = s + 1;
	int c, i;

	/* jump the comment? */
	if ((s[2] == '-') && (s[3] == '-')) {
	  if (!kill_comments)
	    stputs ("<!", out);
	  s += 2;
	  for (;;) {
	    if (strncmp (s, "-->", 3) == 0) {
	      if (!kill_comments)
		stputs ("-->", out);
	      s += 2;
	      break;
	    }
	    else if (*s == 0) {
	      if (!stgets (buf, MAX_BYTES, in))
		break;
	      s = buf;
	    }
	    else {
	      if (!kill_comments)
		stputc (*s, out);
	      s++;
	    }
	  }
	  continue;
	}

	/* jump nested tags */
	for (c = 0;; s++) {
	  if (*s == '<')
	    c++;
	  else if (*s == '>') {
	    c--;
	    if (c == 0)
	      break;
	  }
	  else if (*s == 0) {
	    if (!stgets (buf + strlen (buf), MAX_BYTES - strlen (buf), in))
	      break;
	    s--;
	  }
	}

	c = *s;
	*s = 0;

	/* check for <!end...> */
	if (strncmp (tag + 1, "end", 3) == 0) {
	  current_col = s + 1;
	  done = TRUE;
	  break;
	}
	/* all other tags go to function */
	else {
	  stputc ('<', out);
	  stputs (tag, out);
	  stputc ('>', out);
	}

	if (!c)
	  s--;
	else
	  *s = c;
      }
      /* put a character in the output file */
      else
	stputc (*s, out);
    }

    if (done)
      break;

    current_col = buf;
  } while (stgets (buf, MAX_BYTES, in));
}

static char *tag_function (int argc, char *argv[])
{
  if (argc >= 1) {
    MACRO *macro, *submacro;
    STREAM *stream;
    char buf[32];
    int c;

    /* remove the macro */
    remove_macro (macros_space[nmacros_space-1], argv[0]);

    /* create the new functional macro */
    macro = new_macro (FUNCTIONAL_MACRO, strdup (argv[0]), NULL);

    /* create macros for the function */
    for (c = 1; c < argc; c++) {
      sprintf (buf, "%d", c);

      submacro = new_macro (NORMAL_MACRO, strdup (argv[c]), strdup (""));

      add_macro (macro->data, submacro, FALSE);
    }

    /* read the function stream */
    stream = stopen (NULL, NULL);
    read_function_body (_i_stream, stream);
    macro->value = stbuffer (stream);
    if (!macro->value)
      macro->value = strdup ("");

    add_macro (macros_space[nmacros_space-1], macro, TRUE);
  }
  return NULL;
}

static char *tag_end (int argc, char *argv[])
{
  /* this tag must be used inside <!function> */

  log_printf (0, "you use <!end> outside a <!function>\n");
  return NULL;
}

/* conditional blocks */

/* returns TRUE if `s' represents a number */
static int is_number (char *s)
{
  int r = TRUE;
  /* with sign */
  if ((*s == '+') || (*s == '-'))
    s++;
  /* octal or hexadecimal */
  if (*s == '0') {
    s++;
    /* hexadecimal */
    if (*s == 'x') {
      for (s++; *s; s++)
	if (((*s < '0') || (*s > '9')) &&
	    ((*s < 'a') || (*s > 'f')) && ((*s < 'A') || (*s > 'F'))) {
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

static int eval_if (int argc, char *argv[])
{
  int r = FALSE;
  /* one argument */
  if (argc == 1) {
    /* number */
    if (is_number (argv[0]))
      r = (strtol (argv[0], NULL, 0)) ? TRUE : FALSE;
    /* text */
    else
      r = (*argv[0]) ? TRUE : FALSE;
  }
  /* two arguments */
  else if (argc == 2) {
    if (strcmp (argv[0], "!") == 0) {
      /* number */
      if (is_number (argv[0]))
	r = (strtol (argv[0], NULL, 0)) ? FALSE : TRUE;
      /* text */
      else
	r = (*argv[1]) ? FALSE : TRUE;
    }
  }
  /* three arguments */
  else if (argc == 3) {
    /* number */
    if (is_number (argv[0]) && is_number (argv[2])) {
      int x, y;
      x = strtol (argv[0], NULL, 0);
      y = strtol (argv[2], NULL, 0);
      if (strcmp (argv[1], "==") == 0)
	r = (x == y);
      else if (strcmp (argv[1], "!=") == 0)
	r = (x != y);
      else if (strcmp (argv[1], "<") == 0)
	r = (x < y);
      else if (strcmp (argv[1], ">") == 0)
	r = (x > y);
      else if (strcmp (argv[1], "<=") == 0)
	r = (x < y);
      else if (strcmp (argv[1], ">=") == 0)
	r = (x > y);
    }
    /* text */
    else {
      if (strcmp (argv[1], "==") == 0)
	r = (strcmp (argv[0], argv[2]) == 0) ? TRUE : FALSE;
      else if (strcmp (argv[1], "!=") == 0)
	r = (strcmp (argv[0], argv[2]) != 0) ? TRUE : FALSE;
    }
  }
  /* return the result */
  return r;
}

static char *iftag_if (int argc, char *argv[])
{
  char *new_argv[MAX_ARGS];
  int c;
  /* process all arguments */
  for (c = 0; c < argc; c++)
    new_argv[c] = process_text (argv[c]);
  /* evaluate the expression */
  if (eval_if (argc, new_argv))
    /* for a true expression: we are now inside the if-block */
    new_token (TOK_IF_INSIDE);
  else
    /* the evaluation give false: don't enter in this if-block */
    new_token (TOK_IF_NOTYET);
  /* update parser state */
  update_state ();
  /* free all arguments */
  for (c = 0; c < argc; c++)
    free (new_argv[c]);
  /* nothing to add */
  return NULL;
}

static char *iftag_elif (int argc, char *argv[])
{
  char *new_argv[MAX_ARGS];
  int c;
  /* process all arguments */
  for (c = 0; c < argc; c++)
    new_argv[c] = process_text (argv[c]);
  /* evaluate the expression */
  if (eval_if (argc, new_argv)) {
    /* for a true expression: if we not yet pass for any if-block */
    if (token[0] == TOK_IF_NOTYET)
      /* enter in this block */
      token[0] = TOK_IF_INSIDE;
    /* else, we must jump this block */
    else
      token[0] = TOK_IF_OUTSIDE;
  }
  else {
    /* the evaluation give false: if we are inside in previous if-block */
    if (token[0] == TOK_IF_INSIDE)
      /* now we need to jump all to the next `fi' (end of the if-block) */
      token[0] = TOK_IF_OUTSIDE;
  }
  /* update parser state */
  update_state ();
  /* free all arguments */
  for (c = 0; c < argc; c++)
    free (new_argv[c]);
  /* nothing to add */
  return NULL;
}

static char *iftag_else (int argc, char *argv[])
{
  /* if we not yet pass for any if-block */
  if (token[0] != TOK_IF_OUTSIDE) {
    /* if all previous `if' and `elif' give false */
    if (token[0] == TOK_IF_NOTYET)
      /* now, is time to pass for the `else' */
      token[0] = TOK_IF_INSIDE;
    /* if some previous `if' or `elif' give true */
    else
      /* we need to jump the `else' block */
      token[0] = TOK_IF_OUTSIDE;
  }
  /* update parser state */
  update_state ();
  return NULL;
}

static char *iftag_fi (int argc, char *argv[])
{
  /* back to the old if's state */
  delete_token ();
  /* update the if's states */
  update_state ();
  return NULL;
}

/* lists of tags */

static TAG all_tags[] = {
  /* standard tags */
  {"basename", tag_basename, FALSE},
  {"chop", tag_chop, FALSE},
  {"clean", tag_clean, FALSE},
  {"dep", tag_dep, FALSE},
  {"dir", tag_dir, FALSE},
  {"exec", tag_exec, FALSE},
  {"exec-proc", tag_exec_proc, FALSE},
  {"file-size", tag_file_size, FALSE},
  {"find", tag_find, FALSE},
  {"include", tag_include, FALSE},
  {"macro", tag_macro, FALSE},
  {"macro-reset", tag_macro_reset, FALSE},
  {"notdir", tag_notdir, FALSE},
  {"shift", tag_shift, FALSE},
  {"suffix", tag_suffix, FALSE},
  {"tolower", tag_tolower, FALSE},
  {"toupper", tag_toupper, FALSE},
  {"version", tag_version, FALSE},
  /* function related */
  {"function", tag_function, FALSE},
  {"end", tag_end, FALSE},
  /* special tags */
  {"if", iftag_if, TRUE},
  {"elif", iftag_elif, TRUE},
  {"else", iftag_else, TRUE},
  {"fi", iftag_fi, TRUE},
};

TAG *tags = all_tags;
int ntags = sizeof (all_tags) / sizeof (TAG);
