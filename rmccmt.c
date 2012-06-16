/*
 * Name:	rmccmt.c
 * Purpose:	Remove comments from C and C++ programs. Optionally removes
 *  	    	code and leaves the comment instead.
 * Author:	Lars Wirzenius
 * Version:	"@(#)liwc:$Id: rmccmt.c,v 1.3 2003/01/18 22:37:58 liw Exp $"
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
static int rmcode;

static void print_cmt(int c)
{
    if (rmcode)
    	putchar(c);
}

static void print_code(int c)
{
    if (!rmcode)
    	putchar(c);
}

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
				print_code(c);
				break;
			case '\'':
				st = chr;
				print_code(c);
				break;
			default:
				print_code(c);
				break;
			}
			break;

		case begcmt:
			switch (c) {
			case '/':
				st = incppcmt;
				print_cmt('/');
				print_cmt('/');
				break;
			case '*':
				st = incmt;
				print_cmt('/');
				print_cmt('*');
				break;
			case '"':
				st = str;
				print_code('/');
				print_code(c);
				break;
			case '\'':
				st = chr;
				print_code('/');
				print_code(c);
				break;
			default:
				print_code('/');
				st = code;
				print_code(c);
				break;
			}
			break;

		case incmt:
			print_cmt(c);
			if (c == '\n' && keepnl && !rmcode)
				putchar('\n');
			if (c == '*')
				st = endcmt;
			break;

		case endcmt:
			print_cmt(c);
			if (c == '\n' && keepnl && !rmcode)
				putchar('\n');
			if (c == '/') {
				st = code;
				if (rmcode)
				    putchar('\n'); /* next cmt on new line */
				else
				    putchar(' '); /* replace cmt with space */
			} else if (c != '*')
				st = incmt;
			break;

		case incppcmt:
		    	print_cmt(c);
			if (c == '\n') {
				st = code;
				if (!rmcode)
				    putchar('\n');
			}
			break;

		case str:
			print_code(c);
			if (c == '\\')
				st = strquote;
			else if (c == '"')
				st = code;
			break;

		case strquote:
			print_code(c);
			st = str;
			break;

		case chr:
			print_code(c);
			if (c == '\\')
				st = chrquote;
			else if (c == '\'')
				st = code;
			break;

		case chrquote:
			print_code(c);
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
	
	while (argc > 1 && argv[1][0] == '-') {
	    	if (strcmp(argv[1], "-n") == 0)
		    keepnl = 1;
		else if (strcmp(argv[1], "-c") == 0)
		    rmcode = 1;
		--argc;
		++argv;
	}
	if (main_filter(argc-1, argv+1, rmcmt, NULL) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
