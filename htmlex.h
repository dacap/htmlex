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

#ifndef __HTMLEX_H__
#define __HTMLEX_H__

struct STREAM;
struct MACRO_LIST;

/* version of the program */
#define VERSION "0.4.2"

/* classic macros */
#ifndef NULL
# define NULL 0
#endif
#define FALSE 0
#define TRUE  1

/* determine is the characer is a blank space */
#define IS_BLANK(chr) (((chr) ==  ' ') ||  \
		       ((chr) == '\t') || \
		       ((chr) == '\n') || \
		       ((chr) == '\r'))

#define MAX_FILES 256
#define MAX_ARGS 256
#define MAX_PATHS 256

/* name of the executable */
extern char *htmlex_name;

/* arguments for the input file */
extern char *args[MAX_ARGS];
extern int nargs;

/* the current streams */
extern struct STREAM *_i_stream;
extern struct STREAM *_o_stream;

/* current line */
#define MAX_BYTES 4096

extern char *current_line;
extern char *current_col;

/* last path utiliced by the `try_fopen' function */
extern char success_path[256];

/* states of the parser */
enum {
  TOK_SPACE,			/* independent block space */
  TOK_IF_NOTYET,		/* not yet enter in any if-block */
  TOK_IF_INSIDE,		/* inside the if-block */
  TOK_IF_OUTSIDE,		/* outside the if-block (but we already pass for an if-block) */
};

/* parser tokens */
#define MAX_TOKENS 256
extern char token[MAX_TOKENS];
extern int ntoken;

/* does user want comments? */
extern int kill_comments;

/* some functions */
struct STREAM *try_sopen (const char *filename, const char *mode);
int get_filesize (const char *filename);
char *temp_filename (void);
char *own_strtok (char *s);
void process_file (struct STREAM *in, struct STREAM *out);
char *process_text (const char *s);
void new_token (int type);
void delete_token (void);
void update_state (void);
void add_deps (const char *s);
void PRINTF (int level, const char *format, ...);

#endif				/* __HTMLEX_H__ */
