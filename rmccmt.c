/*
 * Name:	rmccmt.c
 * Purpose:	Remove comments from C and C++ programs.
 * Author:	Lars Wirzenius
 * Version:	"@(#)liwc:$Id: rmccmt.c,v 1.1.1.1 1996/09/16 18:19:51 liw Exp $"
 */

#include <stdio.h>
#include <stdlib.h>
#include <publib.h>

enum state { 
	code, 		/* In code, i.e., outside anything else */
	begcmt, 	/* '/' has been read, possible beginning of comment */
	incmt, 		/* Inside trad. C style comment */
	endcmt, 	/* '*' has been read, possible end of comment */
	incppcmt, 	/* Inside C++ style comment */
	str, 		/* Inside string literal */
	strquote, 	/* Backslash-escaped character inside str. literal */
	chr, 		/* Inside character literal */
	chrquote 	/* Backslash-escaped character inside char. literal */
};

static int keepnl;

static int rmcmt(FILE *f, char *filename, void *dummy) {
	enum state st;
	int c;

	st = code;
	while ((c = getc(f)) != EOF) {
		switch (st) {
		case code:
			switch (c) {
			case '/':
				st = begcmt;
				break;
			case '"':
				st = str;
				putchar(c);
				break;
			case '\'':
				st = chr;
				putchar(c);
				break;
			default:
				putchar(c);
				break;
			}
			break;

		case begcmt:
			switch (c) {
			case '/':
				st = incppcmt;
				break;
			case '*':
				st = incmt;
				break;
			case '"':
				st = str;
				putchar('/');
				putchar(c);
				break;
			case '\'':
				st = chr;
				putchar('/');
				putchar(c);
				break;
			default:
				putchar('/');
				st = code;
				putchar(c);
				break;
			}
			break;

		case incmt:
			if (c == '\n' && keepnl)
				putchar('\n');
			if (c == '*')
				st = endcmt;
			break;

		case endcmt:
			if (c == '\n' && keepnl)
				putchar('\n');
			if (c == '/') {
				st = code;
				putchar(' '); /* replace cmt with space */
			} else if (c != '*')
				st = incmt;
			break;

		case incppcmt:
			if (c == '\n') {
				st = code;
				putchar('\n');
			}
			break;

		case str:
			putchar(c);
			if (c == '\\')
				st = strquote;
			else if (c == '"')
				st = code;
			break;

		case strquote:
			putchar(c);
			st = str;
			break;

		case chr:
			putchar(c);
			if (c == '\\')
				st = chrquote;
			else if (c == '\'')
				st = code;
			break;

		case chrquote:
			putchar(c);
			st = chr;
			break;
		}
	}

	if (st != code) {
		errormsg(0, 0, 
			"input is malformed, ends inside comment or literal");
		return -1;
	}
	
	if (fflush(stdout) == EOF || ferror(stdout))
		return -1;
	return 0;
}


int main(int argc, char **argv) {
	set_progname(argv[0], "rmccmt");
	
	while (argc > 1 && strcmp(argv[1], "-n") == 0) {
		keepnl = 1;
		--argc;
		++argv;
	}
	if (main_filter(argc-1, argv+1, rmcmt, NULL) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
