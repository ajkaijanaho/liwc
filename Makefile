#    Part of liwc, a collection of tools for manipulating C source code
#    Copyright (c) 2012 Antti-Juhani Kaijanaho
#    Copyright (c) 1994-2003 Lars Wirzenius
#
#    This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License,
#    version 2, as published by the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
CC=gcc
CPPFLAGS= -D_POSIX_C_SOURCE=2
CFLAGS=-Wall -ansi -pedantic -O2
LDFLAGS=
LIBS=-lpub

bindir=/usr/local/bin
man1dir=/usr/local/man/man1

bins=ccmtcnvt entrigraph untrigraph rmccmt cstr
scripts=chktri

all: $(bins)

install: all
	install -d $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(man1dir)
	install $(bins) $(scripts) $(DESTDIR)$(bindir)
	install -m 0644 *.1 $(DESTDIR)$(man1dir)

cstr: cstr.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
ccmtcnvt: ccmtcnvt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
rmccmt: rmccmt.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
entrigraph: entrigraph.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
untrigraph: untrigraph.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f core *.o $(bins)
