/*
  utpdf/utps
  margin-aware converter from utf-8 text to PDF/PostScript

  Copyright (c) 2021 by Akihiro SHIMIZU

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

int nbytechar(char c){
    if ((c & 0x80) == 0x00) {
	// 0x00 - 0x7F
	return 1;
    } else if ((c & 0xE0) == 0xC0) {
	// 0xC0 - 0xDF
	return 2;
    } else if ((c & 0xF0) == 0xE0) {
	// 0xE0 - 0xEF
	return 3;
    } else if ((c & 0xF8) == 0xF0) {
	// 0xF0 - 0xF7
	return 4;
    } else if ((c & 0xFC) == 0xF8) {
	// 0xF8 - 0xFB
	return 5;
    } else if ((c & 0xFE) == 0xFC) {
	// 0xFC - 0xFD
	return 6;
    } else {
	return 1;
    }
}

// open file descriptor
int openfd(const char *path, int flag){
    int fd=open(path, flag, 0666);
    char ebuf[256];
    
    if (fd < 0) {
	if ((flag & O_CREAT) != 0) {
	    snprintf(ebuf, 255, "Could not create/write: %s\n", path);
	} else {
	    snprintf(ebuf, 255, "Could not open: %s\n", path);
	}
	perror(ebuf);
	exit(1);
    }
    return fd;
}


//
// UFILE: utf-8 reading interface

UFILE *open_u(char *path) {
    char ebuf[256];
    int fd=open(path, O_RDONLY);

    if (fd < 0){
        snprintf(ebuf, 255, "Could not open for read: %s\n", path);
        perror(ebuf);
        exit(1);
    }
    return fdopen_u(fd, path);
}
    
UFILE *fdopen_u(int fd, char *path){
    UFILE *f;

    f = malloc(sizeof(UFILE));
    f->fd = fd;
    f->eof = 0;
    f->qindex = 0;
    f->lastr = 0;
    f->sindex = 0;
    f->fname = path;
    return f;
}

int close_u(UFILE *f){
    int result;
    result=close(f->fd);
    free(f);
    return result;
}

int get_one_uchar(UFILE *f, char *dst){
    int i, clen;
    char ebuf[256];

    // read from stack
    if (f->sindex>0) {
        clen = nbytechar(f->stack[f->sindex]);
        if (clen <= f->sindex) {
            for (i=0; i<clen; i++) {
                dst[i] = f->stack[--(f->qindex)];
            }
            dst[clen]='\0';
            return clen;
        } else {
            fprintf(stderr, "Something wrong in read stack.\n");
            return 0;
        }
    }
    // read from queue
    if ((f->lastr - f->qindex) >= 1) {
        clen = nbytechar(f->queue[f->qindex]);
        if ((f->lastr - f->qindex) >= clen){
            for (i=0; i<clen; i++) {
                dst[i] = f->queue[(f->qindex)++];
            }
            dst[clen]='\0';
            return clen;
        }
    }
    if (f->eof) {
        return 0;
    }

    {
        char *q;
        int rlen;
        
        // move the rest characters to the first queue.
        q = f->queue;
        for (i = f->qindex; i < f->lastr; i++) {
            *q++ = f->queue[i];
        }
        f->lastr -= f->qindex;
        f->qindex = 0;
        
        // read from file
        rlen = read(f->fd, q, UBUFLEN - f->lastr);
        if (rlen < 0) {
            snprintf(ebuf, 255, "Could not read: %s\n", f->fname);
            perror(ebuf);
            exit(1);
        }
        f->lastr += rlen;
        f->eof=(rlen==0);
        clen = nbytechar(f->queue[f->qindex]);
        if ((f->lastr - f->qindex)>=clen) {
            for (i=0; i<clen; i++) {
                dst[i] = f->queue[(f->qindex)++];
            }
            dst[clen]='\0';
            return clen;
        } else {
            return 0;
        }
    }
}

int rewind_u(UFILE *f, int len){
    if (len <= f->qindex) {
        f->qindex -= len;
        return len;
    } else {
        f->qindex = 0;
        return -1;
    }
}

int eof_u(UFILE *f){
    return (f->eof
            && (f->qindex == f->lastr)
            && (f->sindex == 0));
}

// write to file
cairo_status_t write_func (void *closure, const unsigned char *data,
                           unsigned int length){
    int fd = *(int *)closure;
    int bytes;

    while (length > 0){
        bytes=write(fd, data, length);
        if (bytes<0) return CAIRO_STATUS_WRITE_ERROR;
        length -= bytes;
    }
    return CAIRO_STATUS_SUCCESS;
}

#define PS_END_C "%%EndComments"
#define PS_DUPLEX "<</Duplex true /Tumble false>> setpagedevice\n"

cairo_status_t write_ps_duplex(void *closure, const unsigned char *data,
                               unsigned int length){
    static unsigned char line[1024]="";
    static unsigned int i=0;
    unsigned int p=0;
    static int finished=0;
    int end_of_line;
    
    if (finished) return write_func(closure, data, length);
    
    while ((p<length) && !finished){
        if (data[p] == 0x0D){
            line[i++]=data[p++];
            if (data[p] == 0x0A){
                line[i++]=data[p++];
            }
            end_of_line=1;            
        } else if (data[p]==0x0A){
            line[i++]=data[p++];
            end_of_line=1;            
        } else {
            line[i++]=data[p++];
            end_of_line=0;            
        }
        if (end_of_line){
            if (write_func(closure, line, i) != CAIRO_STATUS_SUCCESS)
                return CAIRO_STATUS_WRITE_ERROR;
            i=0;
            if (bcmp(line, PS_END_C, strlen(PS_END_C))==0){
                if (write_func(closure, (unsigned char *)PS_DUPLEX, strlen(PS_DUPLEX)) != CAIRO_STATUS_SUCCESS)
                    return CAIRO_STATUS_WRITE_ERROR;
                finished=1;
            }
        }
    } // while(p<length)

    if ((finished)&&(p<length)){
        return write_func(closure, &data[p], length-p);
    }

    return CAIRO_STATUS_SUCCESS;
}

// end of io.c

