CC = clang
CFLAGS = -g -Wall -Wextra -std=gnu99
LDFLAGS = -lm

PCMAIN = `pkg-config pangocairo --cflags --libs`
PCOBJ  = `pkg-config pangocairo --cflags`

OBJECTS = drawing.o coord.o io.o usage.o paper.o args.o pangoprint.o

BINDIR = /usr/local/bin
MANDIR = /usr/local/share/man
INSTALL = install -b -C
LN = ln -f

all: utpdf
	$(LN) utpdf utps

install: utpdf
	$(INSTALL) utpdf $(BINDIR)
	$(LN) $(BINDIR)/utpdf $(BINDIR)/utps
	$(INSTALL) ../docs/utpdf.1 $(MANDIR)/man1
	$(LN) $(MANDIR)/man1/utpdf.1 $(MANDIR)/man1/utps.1

utpdf: utpdf.c utpdf.h paper.h drawing.h args.h ${OBJECTS}
	$(CC) $(CFLAGS) $(PCMAIN) ${LDFLAGS} -o $@ ${OBJECTS} $@.c

$(OBJECTS):%.o:%.c
	$(CC) $(CFLAGS) $(PCOBJ) -c -o $@ $<

drawing.o: drawing.c drawing.h coord.h utpdf.h io.h args.h pangoprint.h
coord.o: coord.c coord.h utpdf.h args.h
io.o: io.c io.h
usage.o: usage.c usage.h utpdf.h paper.h args.h
paper.o: paper.c paper.h
args.o: args.c args.h utpdf.h

clean:
	rm -rf *~ *.o *.dSYM a.out

realclean: clean
	rm -rf utpdf utps $(TEST_PROGS)

# ------- for debugging ------- #

TEST_PROGS = pangoprint usage

$(TEST_PROGS):%:%.c
	$(CC) $(CFLAGS) $(PCMAIN) ${LDFLAGS} -DSINGLE_DEBUG -o $@ $<

pagoprint: pangoprint.h pangoprint.c
usage: usage.c usage.h utpdf.h paper.h


# ------- end of Makefile ------- #

