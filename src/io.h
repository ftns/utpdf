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

#ifndef __IO_H__
#define __IO_H__

#include <cairo.h>

#define UBUFLEN   16384 // 16Kbyte
#define USTACKLEN 256

typedef struct utf8_file {
   int fd;
   char queue[UBUFLEN];
   char stack[USTACKLEN];
   char *fname;
   int eof;
   int qindex, lastr, sindex;
} UFILE;

extern int nbytechar(char c);
extern int openfd(const char *path, int flag);

extern UFILE *open_u(char *path);
extern UFILE *fdopen_u(int fd, char *path);
extern int close_u(UFILE *f);
extern int get_one_uchar(UFILE *f, char *dst);
extern int rewind_u(UFILE *f, int len);
extern int eof_u(UFILE *f);

extern cairo_status_t write_func(void *closure, const unsigned char *data, unsigned int length);
extern cairo_status_t write_ps_duplex(void *closure, const unsigned char *data, unsigned int length);


#endif
