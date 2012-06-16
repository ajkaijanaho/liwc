CC=gcc
CFLAGS=-Wall -ansi -pedantic -O2  -D_POSIX_C_SOURCE=2
LDFLAGS=
LIBS=-lpub

bindir=/usr/local/bin
man1dir=/usr/local/man/man1

bins=ccmtcnvt entrigraph untrigraph rmccmt
scripts=chktri

all: $(bins)

install: all
	install $(bins) $(scripts) $(bindir)
	install -m 0644 *.1 $(man1dir)

ccmtcnvt: ccmtcnvt.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
rmccmt: rmccmt.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
entrigraph: entrigraph.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
untrigraph: untrigraph.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f core *.o $(bins)
