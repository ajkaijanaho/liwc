/*
 * ccstr.c -- print out C string literals
 * Lars Wirzenius
 * "@(#)liwc:$Id: cstr.c,v 1.1 1996/09/22 12:26:26 liw Exp $"
 *
 * Usage:	cstr [-hv] [--help] [--version] [file...]
 * All output is to the standard output.
 */

#include <assert.h>
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
#define DFL	UCHAR_MAX+1	/* any character */
#define NONE	UCHAR_MAX+2	/* don't print any character */
#define SELF	UCHAR_MAX+3	/* print last input character */

/*
 * Possible states for the FSM.
 */
enum state { 
	code, chr_lit, chr_esc, str_lit, str_esc, slash, c_cmt, star
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
};

/*
 * The FSM itself.
 */
static const struct rule fsm[] = {
	{ code,	   '"',  str_lit, NONE,	NONE, NONE, NONE	},
	{ code,    '\'', chr_lit, NONE,	NONE, NONE, NONE	},
	{ code,	   '/',  slash,   NONE,	NONE, NONE, NONE	},
	{ code,	   DFL,  code,    NONE,	NONE, NONE, NONE	},
			   
	{ str_lit, '\\', str_esc, SELF,	NONE, NONE, NONE	},
	{ str_lit, '"',  code,    '\n',	NONE, NONE, NONE	},
	{ str_lit, DFL,  str_lit, SELF,	NONE, NONE, NONE	},
			   
	{ str_esc, DFL,  str_lit, SELF,	NONE, NONE, NONE	},
			   
	{ chr_lit, '\\', chr_esc, NONE,	NONE, NONE, NONE	},
	{ chr_lit, '\'', code,    NONE,	NONE, NONE, NONE	},
	{ chr_lit, DFL,  chr_lit, NONE,	NONE, NONE, NONE	},
			   
	{ chr_esc, DFL,  chr_lit, NONE,	NONE, NONE, NONE	},
			   
	{ slash,   '/',  slash,   NONE,	NONE, NONE, NONE	},
	{ slash,   '*',  c_cmt,   NONE,	NONE, NONE, NONE	},
	{ slash,   DFL,  code,    NONE,	NONE, NONE, NONE	},

	{ c_cmt,   '*',  star,    NONE, NONE, NONE, NONE	},
	{ c_cmt,   DFL,  c_cmt,   NONE, NONE, NONE, NONE	},

	{ star,    '/',  code,    NONE, NONE, NONE, NONE	},
	{ star,    '*',  star,    NONE, NONE, NONE, NONE	},
	{ star,    DFL,  c_cmt,   NONE, NONE, NONE, NONE	},
};




static int cstr(FILE *f, char *filename, void *);

static const char usage[] = "[-hv] [file ...]\n";
static const char version[] = "version 1.0";


/*
 * Do the usual main() stuff: interpret options, process all input files.
 */
int main(int argc, char **argv) {
	int opt;

	set_progname(argv[0], "cstr");

	while ((opt = getopt(argc, argv, "hv")) != EOF) {
		switch (opt) {
		case 'h':
		case '?':
			errormsg(1, 0, "%s", usage);
		case 'v':
			errormsg(1, 0, "%s", version);
		}
	}
	
	if (main_filter(argc-optind, argv+optind, cstr, NULL) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}


/*
 * Process one file.  Output is to stdout.  Return 0 for OK, -1 for error.
 */

#define print(x) (void)((x) != NONE && printf("%c", (x) == SELF ? c : (x)))

static int cstr(FILE *f, char *filename, void *dummy) {
	enum state state;
	const struct rule *p;
	int c, i;

	state = code;
	for (;;) {
		c = getc(f);
		if (c == EOF) {
			if (ferror(f)) {
				errormsg(0, -1, "error reading %s\n");
				return -1;
			}

			if (state != code) {
				errormsg(0, 0, "%s ends funnily\n", filename);
				return -1;
			}
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
				break;
			}
		}
	}
}
