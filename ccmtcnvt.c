/*
 * Part of liwc, a collection of tools for manipulating C source code
 * Copyright (c) 2012 Antti-Juhani Kaijanaho
 * Copyright (c) 1994-2003 Lars Wirzenius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
/*
 * ccmtcnvt.c -- convert C++ comments to C comments
 * Lars Wirzenius
 *
 * Usage:	ccmtcnvt [-hv] [--help] [--version] [file...]
 * All output is to the standard output.
 *
 * This  program  converts the C++ style comments into traditional C style
 * comments.   C++  allows  comments  of  that begin  with  `//'  and end
 * with the next newline, but they weren't introduced to the C language
 * until the  1999  version  of the C standard.  Some C compilers alre3ady
 * accept these, but for those who don't, you can use  this  utility to
 * convert them.
 * 
 * This is written as a finite state machine (FSM), which reads (presumably
 * correct) C code, and when it finds a // that is not inside a comment, or
 * string or character literal, it converts it to a C start of comment and
 * adds a space and a comment delimiter (unreproducible here :-) before the
 * next newline.
 *
 * The command line options require POSIX 1003.2 to work, because they
 * assume that getopt is declared by <unistd.h>.
 *
 * Known problems: 
 *
 * a) Include file names are not recognized being special, so that
 *	
 *	#include <sys//stat.h>
 *
 * will be mangled.  I've never seen such names used in source code, though,
 * so I don't think ignoring them will cause too much harm.
 *
 * b) Does not understand trigraphs or digraphs.
 *
 * c) Does not understand line continuation (\ at end of line) outside
 * string literals and character constants.
 *
 * Patches for fixing these are welcome. They're not common enough for me
 * to worry about them, at least for now.
 */

/*
 * Neat trick dept.: the following expression will evaluate to 0 if
 * C++ comments are not supported, and 1 if they are.  (x is here *;
 * can't use * because it would terminate the comment.)
 *
 *	int cppcmts = 1//xx/2
 *		;
 *
 * or use it like this:
 *
 *	#if 1//xx/2
 *	#error This C compiler is confused about comment syntax
 *	#endif
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <publib.h>


/*
 * Special marks for input characters used in the description of the FSM.
 */
#define DFL	(int)UCHAR_MAX+1	/* any character */
#define NONE	(int)UCHAR_MAX+2	/* don't print any character */
#define SELF	(int)UCHAR_MAX+3	/* print last input character */

/*
 * Possible states for the FSM.
 */
enum state { 
	code, chr_lit, chr_esc, str_lit, str_esc, 
	slash, cpp_cmt, cpp_ast, c_cmt, star
};

/*
 * Rules for describing the state changes and associated actions for the
 * FSM.
 */
struct rule {
	enum state state;
	int c;
	enum state new_state;
	int print1, print2, print3, print4;
	const char *warning;
};

/*
 * The FSM itself.
 */
static const struct rule fsm[] = {
	{ code,	   '"',  str_lit, SELF,	NONE, NONE, NONE, NULL	},
	{ code,    '\'', chr_lit, SELF,	NONE, NONE, NONE, NULL	},
	{ code,	   '/',  slash,   NONE,	NONE, NONE, NONE, NULL	},
	{ code,	   DFL,  code,    SELF,	NONE, NONE, NONE, NULL	},
			   
	{ str_lit, '\\', str_esc, SELF,	NONE, NONE, NONE, NULL	},
	{ str_lit, '"',  code,    SELF,	NONE, NONE, NONE, NULL	},
	{ str_lit, DFL,  str_lit, SELF,	NONE, NONE, NONE, NULL	},
			   
	{ str_esc, DFL,  str_lit, SELF,	NONE, NONE, NONE, NULL	},
			   
	{ chr_lit, '\\', chr_esc, SELF,	NONE, NONE, NONE, NULL	},
	{ chr_lit, '\'', code,	  SELF,	NONE, NONE, NONE, NULL	},
	{ chr_lit, DFL,	 chr_lit, SELF,	NONE, NONE, NONE, NULL	},
			   
	{ chr_esc, DFL,	 chr_lit, SELF,	NONE, NONE, NONE, NULL	},
			   
	{ slash,   '/',	 cpp_cmt, '/',	'*',  NONE, NONE, NULL	},
	{ slash,   '*',	 c_cmt,	  '/',	'*',  NONE, NONE, NULL	},
	{ slash,   DFL,	 code,	  '/',	SELF, NONE, NONE, NULL	},

	{ cpp_cmt, '\n', code,	  ' ',	'*',  '/',  SELF, NULL	},
	{ cpp_cmt, '*',	 cpp_ast, SELF,	NONE, NONE, NONE, NULL	},
	{ cpp_cmt, DFL,	 cpp_cmt, SELF, NONE, NONE, NONE, NULL	},

	{ cpp_ast, '\n', code,	  ' ',	'*',  '/',  SELF, NULL	},
	{ cpp_ast, '*',	 cpp_ast, SELF,	NONE, NONE, NONE, NULL	},
	{ cpp_ast, '/',	 cpp_cmt, ' ',	SELF, NONE, NONE,
	  "converted a '*/' inside a C++ comment to '* /'."	},
	{ cpp_ast, DFL,	 cpp_cmt, SELF, NONE, NONE, NONE, NULL	},

	{ c_cmt,   '*',	 star,	  SELF, NONE, NONE, NONE, NULL	},
	{ c_cmt,   DFL,	 c_cmt,	  SELF, NONE, NONE, NONE, NULL	},

	{ star,	   '/',	 code,	  SELF, NONE, NONE, NONE, NULL	},
	{ star,	   '*',	 star,	  SELF, NONE, NONE, NONE, NULL	},
	{ star,	   DFL,	 c_cmt,	  SELF, NONE, NONE, NONE, NULL	},
};


static int convert(FILE *f, char *filename, void *);

static char usage[] = "Usage: %s [-hv] [file ...]\n";
static char version[] = "version 1.1\n";


/*
 * Do the usual main() stuff: interpret options, process all input files.
 */
int main(int argc, char **argv) {
	int opt;

	set_progname(argv[0], "ccmtcnvt");

	while ((opt = getopt(argc, argv, "hv")) != EOF) {
		switch (opt) {
		case 'h':
		case '?':
			errormsg(1, 0, usage, get_progname());
		case 'v':
			errormsg(1, 0, "%s", version);
		}
	}

	if (main_filter(argc-optind, argv+optind, convert, NULL) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

#define print(x) (void)((x) != NONE && printf("%c", (x) == SELF ? c : (x)))
static int convert(FILE *f, char *filename, void *dummy) {
	enum state state;
	const struct rule *p;
	int c, i;

	state = code;
	for (;;) {
		c = getc(f);
		if (c == EOF) {
			if (ferror(f)) {
				errormsg(0, errno, "error reading %s\n");
				return -1;
			}

			switch (state) {
			case cpp_cmt:
				/* need to terminate the last // comment */
				printf(" */");
				break;
			case code:
				/* do nothing 'cause all's good */
				break;
			default:
				errormsg(0, 0, "%s ends funnily\n", filename);
				return -1;
			}
	
			if (fflush(stdout) == EOF || ferror(stdout))
				return -1;
			return 0; /* everything seems to be OK */
		}

		for (i = 0; i < sizeof(fsm) / sizeof(fsm[0]); ++i) {
			p = &fsm[i];
			if (p->state == state && (p->c == c || p->c == DFL)) {
				state = p->new_state;
				print(p->print1);
				print(p->print2);
				print(p->print3);
				print(p->print4);
				if (p->warning != NULL)
					errormsg(0, 0, "%s", p->warning);
				break;
			}
		}
	}
	/*NOTREACHED*/
}
