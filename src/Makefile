CC = clang
CFLAGS = -g -Wall -Wextra -std=gnu99
LDFLAGS = -lm

MAIN_FLAGS = `pkg-config pangocairo --cflags --libs`
OBJ_FLAGS  = `pkg-config pangocairo --cflags`

OBJECTS = drawing.o coord.o io.o usage.o paper.o args.o pangoprint.o

BINDIR = /usr/local/bin
MANDIR = /usr/local/share/man
INSTALL_BIN = install -b -C -s -m 0755
INSTALL_DOC = install -b -C -m 0644
LN = ln -f

all: utpdf
	ln -fs utpdf utps

install: utpdf
	$(INSTALL_BIN) utpdf $(BINDIR)
	$(LN) $(BINDIR)/utpdf $(BINDIR)/utps
	$(INSTALL_DOC) ../docs/utpdf.1 $(MANDIR)/man1
	$(LN) $(MANDIR)/man1/utpdf.1 $(MANDIR)/man1/utps.1

utpdf: utpdf.c utpdf.h paper.h drawing.h args.h ${OBJECTS}
	$(CC) $(CFLAGS) $(MAIN_FLAGS) ${LDFLAGS} -o $@ ${OBJECTS} $@.c

$(OBJECTS):%.o:%.c
	$(CC) $(CFLAGS) $(OBJ_FLAGS) -c -o $@ $<

drawing.o: drawing.c drawing.h coord.h utpdf.h io.h args.h pangoprint.h
coord.o:   coord.c coord.h utpdf.h args.h
io.o:      io.c io.h utpdf.h
usage.o:   usage.c usage.h utpdf.h paper.h args.h
paper.o:   paper.c paper.h
args.o:    args.c args.h utpdf.h
pangoprint.o: pangoprint.h pangoprint.c utpdf.h io.h

clean:
	rm -rf *~ *.o *.dSYM a.out

realclean: clean
	rm -rf utpdf utps $(TEST_PROGS)

# ------- for debugging ------- #

TEST_PROGS = pangoprint usage io

#$(TEST_PROGS):%:%.c
#	$(CC) $(CFLAGS) $(MAIN_FLAGS) ${LDFLAGS} -DSINGLE_DEBUG -o $@ $<

$(TEST_PROGS):%:%.c
	$(CC) $(CFLAGS) $(MAIN_FLAGS) ${LDFLAGS} -DSINGLE_DEBUG  $($@.OBJ) -o $@ $<

pangoprint: pangoprint.h pangoprint.c utpdf.h io.h
pangoprint.OBJ=io.o

usage: usage.c usage.h utpdf.h paper.h
io: io.c io.h

# ------- end of Makefile ------- #

