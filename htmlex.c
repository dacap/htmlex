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

/* the current streams */
STREAM *_i_stream = NULL;
STREAM *_o_stream = NULL;

/* current line */
char *current_line = NULL;
char *current_col = NULL;

/* last path utiliced by the `try_fopen' function */
char success_path[256];

/* parser states */
char token[MAX_TOKENS];
int ntoken = 0;

/* does user want comments? */
int kill_comments = FALSE;

/* we can can attach text in the output */
static int can_attach;
static int can_dep;

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
STREAM *try_sopen (const char *filename, const char *mode)
{
  STREAM *stream;
  strcpy (success_path, filename);
  stream = stopen (success_path, mode);
  if (!stream) {
    int c;
    for (c = 0; c < npaths; c++) {
      sprintf (success_path, "%s/%s", paths[c], filename);
      stream = stopen (success_path, mode);
      if (stream)
	break;
    }
  }
  return stream;
}

/* gets the size of a file */
int get_filesize (const char *filename)
{
  FILE *f;
  int size = 0;
  f = fopen (filename, "rb");
  if (f) {
    fseek (f, 0, SEEK_END);
    size = ftell (f);
    fclose (f);
  }
  return size;
}

char *temp_filename (void)
{
  return strdup ("_tmpfile.out");
}

/* gets the file name of the path */
static const char *get_filename (const char *path)
{
  char *s = strrchr (path, '/');
  return (s) ? s + 1 : path;
}

/* replaces the file extension of the `path' with the new one
   indicated by `extension' */
static char *replace_extension (char *dest, const char *path,
				const char *extension)
{
  char *s = strrchr (get_filename (strcpy (dest, path)), '.');
  if (s)
    *s = 0;
  return strcat (dest, extension);
}

/* gets tokens from the string, the tokens can be separates by spaces
   (or tabs or new lines), and each one can enclosed by quotes or angular
   brackets, example:

   hi, 'good bye' <hi bye> "-\"good bye\"-"

     tok=hi,
     tok=good bye
     tok=<hi bye>
     tok=-"good bye"-
*/
char *own_strtok (char *s)
{
  static char *d = NULL;
  char *r;

  if (s)
    d = s;
  else if (!d)
    return NULL;

  if ((*d != '\"') && (*d != '\'')) {
    for (r = d; !IS_BLANK (*d); d++) {
      if (*d == '<') {
	int c;
	for (c = 0;; d++) {
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
  /* double quote */
  else if (*d == '\"') {
    r = ++d;
    for (;;) {
      if (*d == 0) {
	d = NULL;
	break;
      }
      else if (*d == '\"') {
	break;
      }
      else if (*d == '\\') {
	memmove (d, d+1, strlen (d+1));
	switch (*d) {
	  case 'n': *d = '\n'; break;
	  case 't': *d = '\t'; break;
	  case 'r': *d = '\r'; break;
	}
	d++;
      }
      else
	d++;
    }
  }
  /* single quote */
  else {
    r = ++d;
    for (;;) {
      if (*d == 0) {
	d = NULL;
	break;
      }
      else if (*d == '\'') {
	break;
      }
      else
	d++;
    }
  }

  if (d) {
    *d = 0;

    do {
      d++;
    } while ((*d) && IS_BLANK (*d));
    if (!*d)
      d = NULL;
  }

  return r;
}

/* process the file `in' and output the result to `out' file */
void process_file (STREAM * in, STREAM * out)
{
  STREAM *old_i_stream = _i_stream;
  STREAM *old_o_stream = _o_stream;
  char *old_current_line = current_line;
  char *old_current_col = current_col;
  char *s, buf[MAX_BYTES];

  _i_stream = in;
  _o_stream = out;

  old_current_line = current_line;
  old_current_col = current_col;

  new_token (TOK_SPACE);
  update_state ();

  while (stgets (buf, MAX_BYTES, in)) {
    for (s = buf; *s; s++) {
      /* tag beginning */
      if ((*s == '<') && (s[1] == '!')) {
	int c, i, used = FALSE;
	int restore = TRUE;
	char *tag = s + 1;

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

	PRINTF (2, "tag found: \"%s\"\n", tag + 1);

	/* check for <!arg...> */
	if (strncmp (tag + 1, "arg", 3) == 0) {
	  if (can_attach) {
	    /* <!args...> */
	    if (tag[4] == 's') {
	      char temp[32];
	      sprintf (temp, "%d", nargs);
	      stputs (temp, out);
	    }
	    /* <!arg[1-9][0-9]*...> */
	    else {
	      int arg = strtol (tag + 4, NULL, 10);
	      if ((arg > 0) && (arg <= nargs) && (args[arg - 1]))
		stputs (args[arg - 1], out);
	    }
	  }
	  used = TRUE;
	}
	/* check for built-ins functions <!...> */
	if (!used) {
	  for (i = 0; i < ntags; i++) {
	    if (strncmp (tag + 1, tags[i].name, strlen (tags[i].name)) ==
		0) {
	      int x = tag[1 + strlen (tags[i].name)];
	      if (IS_BLANK (x) || (!x)) {
		char *tok, *argv[MAX_ARGS];
		char *replacement;
		int argc = 0;

		for (tok = own_strtok (tag + 2),
		     tok = own_strtok (NULL); tok; tok = own_strtok (NULL))
		  argv[argc++] = tok;

		if ((tags[i].if_tag) || (can_attach)) {
		  current_line = buf;
		  current_col = s + 1;

		  /* call the tag procedure */
		  replacement = (*tags[i].proc) (argc, argv);

		  if (s != current_col - 1) {
		    s = current_col - 1;
		    restore = FALSE;
		  }

		  /* text to replace */
		  if (replacement) {
		    stputs (replacement, out);
		    free (replacement);
		  }

		  PRINTF (2, "tag \"%s\" was processed\n", tags[i].name);
		}
		else
		  PRINTF (2, "tag \"%s\" wasn't processed\n",
			  tags[i].name);

		used = TRUE;
		break;
	      }
	    }
	  }
	}
	/* check for user functional macros <!...> */
	if (!used) {
	  char *replacement = function_macro (global_macros, tag);
	  if (replacement) {
	    stputs (replacement, out);
	    free (replacement);
	    used = TRUE;
	  }
	}
	/* well, this is an unknown tag */
	if (!used) {
	  char *ptag = process_text (tag);

	  if (can_attach)
	    stputc ('<', out);

	  if (ptag) {
	    if (can_attach)
	      stputs (ptag, out);

	    free (ptag);
	  }

	  if (can_attach)
	    stputc ('>', out);
	}

	if (restore) {
	  if (!c)
	    s--;
	  else
	    *s = c;
	}
      }
      /* put a character in the output file */
      else if (can_attach) {
	char *replacement = NULL;
	int c, length = 0;

	/* check for function macros */
	for (c = 0; c < nfunction_macros; c++)
	  if (function_macros[c]) {
	    replacement =
		replace_by_macro (function_macros[c], s, &length);
	    if (replacement)
	      break;
	  }

	/* check for global macros */
	if (!replacement)
	  replacement = replace_by_macro (global_macros, s, &length);

	/* just put the character */
	if (!replacement) {
	  stputc (*s, out);
	}
	/* put the value of the macro */
	else {
	  stputs (replacement, out);
	  s += length - 1;
	  free (replacement);
	}
      }
    }
  }

  delete_token ();
  update_state ();

  _i_stream = old_i_stream;
  _o_stream = old_o_stream;

  current_line = old_current_line;
  current_col = old_current_col;
}

/* process a block of text like a file */
char *process_text (const char *s)
{
  STREAM *in, *out;
  char *r;
  in = stopen (NULL, NULL);
  out = stopen (NULL, NULL);
  stputs (s, in);
  stseek (in, 0, SEEK_SET);
  process_file (in, out);
  stclose (in);
  r = stbuffer (out);
  return (r) ? r : strdup ("");
}

void new_token (int type)
{
  int c;
  for (c = ntoken; c > 0; c--)
    token[c] = token[c - 1];
  ntoken++;
  token[0] = type;
}

void delete_token (void)
{
  int c;
  for (c = 0; c < ntoken; c++)
    token[c] = token[c + 1];
  ntoken--;
}

void update_state (void)
{
  int c;

  can_attach = TRUE;
  for (c = 0; c < ntoken; c++) {
    if (token[c] == TOK_SPACE)
      break;
    if (token[c] != TOK_IF_INSIDE) {
      can_attach = FALSE;
      break;
    }
  }

  can_dep = TRUE;
  for (c = 0; c < ntoken; c++) {
    if (token[c] == TOK_SPACE)
      continue;
    if (token[c] != TOK_IF_INSIDE) {
      can_dep = FALSE;
      break;
    }
  }
}

void add_deps (const char *s)
{
  if (calculating_deps && can_dep) {
    PRINTF (1, "new dependency: \"%s\"\n", s);

    if (!depstream)
      depstream = stopen (NULL, NULL);

    stputs (s, depstream);
    stputc ('\n', depstream);
  }
}

void out_deps (void)
{
  if (calculating_deps && depstream) {
    char buf[256];
    int x;
    stseek (depstream, 0, SEEK_SET);
    /* file name */
    stgets (buf, sizeof (buf), depstream);
    buf[strlen (buf) - 1] = 0;
    x = printf ("%s:", buf);
    /* dependencies */
    while (stgets (buf, sizeof (buf), depstream)) {
      buf[strlen (buf) - 1] = 0;

      x += strlen (buf);
      if (x > 76) {
	printf (" \\\n");
	x = strlen (buf);
      }
      else
	x += printf (" ");

      printf (buf);
    }
    printf ("\n");
    stclose (depstream);
    depstream = NULL;
  }
}

void PRINTF (int level, const char *format, ...)
{
  if (level <= verbose_level) {
    char buf[1024];
    va_list ap;
    va_start (ap, format);
    vsprintf (buf, format, ap);
    va_end (ap);
    fprintf (stderr, "%s: %s", name, buf);
  }
}

/* shows the program usage information */
static void usage (void)
{
  printf ("\
htmlex %s - a powerful hypertext markup language preprocessor
Copyright (C) 2001, 2002, 2003 by David A. Capello
\n\
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
  -k   kill comments (old htmlex behavior)\n\
  -d   calculates dependencies of the input files (output to STDOUT)\n\
  -v   activates the verbose mode (to see what htmlex does)\n\
  -V   very verbose mode\n\
  -h   displays help screen and exit\n\
  --   terminates a -c, -o, -a or -i list\n\
\n\
Report bugs and patches to <dacap@users.sourceforge.net>\n\
", VERSION);
}

static void prepare_processing (void)
{
  global_macros = new_macro_list ();
  function_macros[0] = NULL;
}

static void release_processing (void)
{
  free_macro_list (global_macros);
}

/* main function */
int main (int argc, char *argv[])
{
#define MAX_FILES 256
  char *files[MAX_FILES];
  int nfiles = 0;
  int process_stdin = TRUE;
  int compile_next = FALSE;
  int output_next = FALSE;
  int argument_next = FALSE;
  int include_next = FALSE;
  int i, j;

  name = argv[0];

  PRINTF (1, "processing arguments...\n");

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      compile_next = FALSE;
      output_next = FALSE;
      argument_next = FALSE;
      include_next = FALSE;

      for (j = 1; argv[i][j]; j++) {
	switch (argv[i][j]) {
	  case 'c':
	    compile_next = TRUE;
	    break;
	  case 'o':
	    output_next = TRUE;
	    break;
	  case 'a':
	    argument_next = TRUE;
	    break;
	  case 'i':
	    include_next = TRUE;
	    break;
	  case 'k':
	    kill_comments = TRUE;
	    break;
	  case 'd':
	    calculating_deps = TRUE;
	    break;
	  case 'v':
	    verbose_level = 1;
	    break;
	  case 'V':
	    verbose_level = 2;
	    break;
	  case 'h':
	    usage ();
	    exit (0);
	    break;
	  case '-':
	    break;
	  default:
	    PRINTF (0, "%s: unknow option\n", argv[i]);
	    exit (1);
	    break;
	}
      }
    }
    /* new output file */
    else if (output_next) {
      files[nfiles++] = argv[i];
      PRINTF (1, "new output file: \"%s\"\n", argv[i]);
    }
    /* new arguments for the input files */
    else if (argument_next) {
      args[nargs++] = argv[i];
      PRINTF (1, "new argument: \"%s\"\n", argv[i]);
    }
    /* new path for inclusion of files */
    else if (include_next) {
      paths[npaths++] = argv[i];
      PRINTF (1, "new path: \"%s\"\n", argv[i]);
    }
    else {
      STREAM *in, *out;
      char buf[256];

      PRINTF (1, "processing \"%s\" file\n", argv[i]);

      /* open the input file */
      in = try_sopen (argv[i], "r");
      if (!in) {
	PRINTF (0, "%s: file not found\n", argv[i]);
	exit (1);
      }

      /* output to file */
      if (compile_next) {
	/* we can get a file name from the output's files */
	if (nfiles > 0) {
	  strcpy (buf, files[0]);

	  nfiles--;
	  for (j = 0; j < nfiles; j++)
	    files[j] = files[j + 1];
	}
	/* auto generate the output file name */
	else {
	  replace_extension (buf, success_path, ".html");
	}

	PRINTF (1, "output to \"%s\" file\n", buf);

	update_state ();

	if (calculating_deps) {
	  add_deps (buf);
	  add_deps (argv[i]);
	  out = NULL;
	}
	else {
	  out = try_sopen (buf, "w");
	  if (!out) {
	    PRINTF (0, "%s: can't create file\n", buf);
	    exit (1);
	  }
	}

	/* "j" will be used for PRINTF () */
	j = i;
      }
      /* output to STDOUT */
      else {
	PRINTF (1, "output to STDOUT\n");

	update_state ();

	add_deps (argv[i]);

	if (calculating_deps)
	  out = NULL;
	else
	  out = stfile (stdout);

	/* "j" will be used for PRINTF () */
	j = i;

	/* all next arguments for the file too */
	for (i++; i < argc; i++)
	  args[nargs++] = argv[i];
      }

      /* process the file */
      prepare_processing ();
      process_file (in, out);
      release_processing ();

      /* close the files */
      stclose (in);
      stclose (out);

      PRINTF (1, "done with \"%s\" file\n", argv[j]);	/* "j" is used here */

      out_deps ();

      process_stdin = FALSE;
    }
  }

  /* we must process STDIN because none file was process yet */
  if ((process_stdin) && (!calculating_deps)) {
    STREAM *in, *out;

    PRINTF (1, "processing STDIN\n");

    in = stfile (stdin);

    if (nfiles > 0) {
      out = try_sopen (files[0], "w");
      if (!out) {
	PRINTF (0, "%s: can't create file\n", files[0]);
	exit (1);
      }
    }
    else
      out = stfile (stdout);

    prepare_processing ();
    process_file (in, out);
    release_processing ();

    stclose (in);
    stclose (out);

    PRINTF (1, "done with STDIN\n");
  }

  PRINTF (1, "all done\n");

  return 0;
}
