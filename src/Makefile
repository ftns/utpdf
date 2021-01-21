CC = clang
CFLAGS = -g -Wall -Wextra -std=c99
LDFLAGS = -lm

PCMAIN = `pkg-config pangocairo --cflags --libs`
PCOBJ  = `pkg-config pangocairo --cflags`

OBJECTS = drawing.o coord.o io.o usage.o paper.o args.o

all: utpdf
	rm -f utps
	ln utpdf utps

utpdf: utpdf.c utpdf.h paper.h drawing.h args.h ${OBJECTS}
	$(CC) $(CFLAGS) $(PCMAIN) ${LDFLAGS} -o $@ ${OBJECTS} $@.c

$(OBJECTS):%.o:%.c
	$(CC) $(CFLAGS) $(PCOBJ) -c -o $@ $<

drawing.o: drawing.c drawing.h coord.h utpdf.h io.h args.h
coord.o: coord.c coord.h utpdf.h args.h
io.o: io.c io.h
usage.o: usage.c usage.h utpdf.h paper.h
paper.o: paper.c paper.h
args.o: args.c args.h utpdf.h

usage: usage.c utpdf.h paper.o
	$(CC) $(CFLAGS) ${PCMAIN} -DSINGLE_DEBUG -o $@ paper.o $@.c
clean:
	rm -rf *~ *.o *.dSYM a.out
realclean: clean
	rm -rf utpdf utps
