/*
 * Part of liwc, a collection of tools for manipulating C source code
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
 * Name:	untrigraph.c
 * Purpose:	Remove trigraphs from C code.
 * Author:	Lars Wirzenius
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <publib.h>

int untrigraph(FILE *, char *, void *);

int main(int argc, char **argv) {
	set_progname(argv[0], "untrigraph");
	if (main_filter(argc-1, argv+1, untrigraph, NULL) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}


/*
 * Name:	untrigraph
 * Purpose:	Remove trigraphs from one file, output to stdout.
 * Description:	A sequence of four characters (c0, c1, c2, c3) is a
 *		trigraph, if c0 != '\\', c1==c2=='?' and c3 is one of the
 *		characters inside the string "()<>!'-=/".
 *
 *		In the code below, c0 is the previous character we printed
 *		(possibly the result of a trigraph conversion).  
 */
int untrigraph(FILE *f, char *fname, void *dummy) {
	int c0, c1, c2, c3;
	char tri[]       = "()<>!'-=/";  /* list of trigraphs... */
	char realchars[] = "[]{}|^~#\\"; /* ...and corresponding chars */
	char *p;

	c0 = c2 = c3 = EOF;
	p = NULL;

	if ((c1=getc(f)) != EOF && (c2=getc(f)) != EOF)
		c3 = getc(f);
	while (c1 != EOF) {
		if (c0 == '\\' || c1 != '?' || c2 != '?' ||
		    (c3 != EOF && (p = strchr(tri, c3)) == NULL)) {
			putchar(c1);
			c0 = c1;
			c1 = c2;
			c2 = c3;
			if (c3 != EOF)
				c3 = getc(f);
		} else {	/* it's a trigraph! */
			assert(p != NULL);
			assert(p >= tri && p < tri + sizeof(tri) - 1);
			c0 = realchars[p-tri];
			putchar(c0);
			c1 = c2 = c3 = EOF;
			if ((c1=getc(f)) != EOF && (c2=getc(f)) != EOF)
				c3 = getc(f);
		}
	}
	if (ferror(f))
		return -1;
	if (fflush(stdout) == EOF || ferror(stdout))
		return -1;
	return 0;
}
