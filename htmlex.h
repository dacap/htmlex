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

#ifndef __HTMLEX_H__
#define __HTMLEX_H__

/* input/output stream */
struct _IO_STREAM;

/* version of the program */
#define VERSION "0.3"

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

/* arguments for the input file */
#define MAX_ARGS 256
extern char *args[MAX_ARGS];
extern int nargs;

/* the active output stream */
extern struct _IO_STREAM *_o_stream;

/* last path utiliced by the `try_fopen' function */
extern char success_path[256];

/* states of the `if' comparations */
#define IF_SPACE    -1 /* if-spaces separator (one space is a independent block of other) */
#define IF_NOTYET    0 /* not yet enter in any if-block */
#define IF_INSIDE    1 /* inside the if-block */
#define IF_OUTSIDE   2 /* outside the if-block (but we already pass for a if-block) */

/* `if' states */
#define MAX_IFS 256
extern char ifs[MAX_IFS];
extern int nifs;

extern int can_attach;

/* some functions */
struct _IO_STREAM *try_sopen(const char *filename, const char *mode);
int get_filesize(const char *filename);
char *temp_filename(void);
void process_file(struct _IO_STREAM *in, struct _IO_STREAM *out);
char *process_text(const char *s);
void new_ifs(int type);
void delete_ifs(void);
void update_ifs(void);
void add_deps(const char *s);

#endif /* __HTMLEX_H__ */

