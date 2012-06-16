/*
 * Name:	entrigraph.c
 * Purpose:	Put trigraphs into C source code file.
 * Author:	Lars Wirzenius
 * Version:	"@(#)liwctools:$Id: entrigraph.c,v 1.1.1.1 1996/09/16 18:19:51 liw Exp $"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <publib.h>

int entrigraph(FILE *f, char *fname, void *dummy) {
	int c;
	char tri[]       = "()<>!'-=/";  /* list of trigraphs... */
	char realchars[] = "[]{}|^~#\\"; /* ...and corresponding chars */
	char *p;

	while ((c = getc(f)) != EOF) {
		if ((p = strchr(realchars, c)) == NULL)
			putchar(c);
		else
			printf("?\?%c", tri[p-realchars]);
	}
	return 0;
}

int main(int argc, char **argv) {
	set_progname(argv[0], "entrigraph");
	if (main_filter(argc-1, argv+1, entrigraph, NULL) == -1)
		exit(EXIT_FAILURE);
	exit(0);
}
