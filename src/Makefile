CC = clang
CFLAGS = -g -Wall -Wextra -std=c99
LDFLAGS = -lm

PCMAIN = `pkg-config pangocairo --cflags --libs`
PCOBJ  = `pkg-config pangocairo --cflags`

all: utpdf
	ln utpdf utps

utpdf: utpdf.c utpdf.h paper.h drawing.h drawing.o coord.o io.o usage.o paper.o
	$(CC) $(CFLAGS) $(PCMAIN) ${LDFLAGS} -o utpdf drawing.o coord.o io.o usage.o paper.o utpdf.c

drawing.o: drawing.c drawing.h coord.h utpdf.h io.h
	$(CC) $(CFLAGS) $(PCOBJ) -c -o drawing.o drawing.c

coord.o: coord.c coord.h utpdf.h
	$(CC) $(CFLAGS) $(PCOBJ) -c -o coord.o coord.c

io.o: io.c io.h
	$(CC) $(CFLAGS) -c -o io.o io.c

usage.o: usage.c utpdf.h paper.h
	$(CC) $(CFLAGS) ${PCOBJ} -c -o usage.o usage.c

usage: usage.c utpdf.h paper.h
	$(CC) $(CFLAGS) ${PCMAIN} -DSINGLE_DEBUG -o usage paper.o usage.c

paper.o: paper.c paper.h
	$(CC) $(CFLAGS) ${PCOBJ} -c -o paper.o paper.c

clean:
	rm -r *~ *.o *.dSYM

realclean:
	rm -r *~ *.o *.dSYM utpdf utps
