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
 * Name:	entrigraph.c
 * Purpose:	Put trigraphs into C source code file.
 * Author:	Lars Wirzenius
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
