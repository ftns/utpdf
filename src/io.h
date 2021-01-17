/*
  margin-aware converter from utf-8 text to PDF/PostScript
  utpdf utps

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

#define UBUFLEN   16384 // 16Kbyte
#define USTACKLEN 256

struct UFILE {
   int fd;
   char queue[UBUFLEN];
   char stack[USTACKLEN];
   char *fname;
   int eof;
   int qindex, lastr, sindex;
};

extern int nbytechar(char c);
extern int openfd(const char *path, int flag);

extern struct UFILE *open_u(char *path);
extern struct UFILE *fdopen_u(int fd, char *path);
extern int close_u(struct UFILE *f);
extern int get_one_uchar(struct UFILE *f, char *dst);
extern int rewind_u(struct UFILE *f, int len);
extern int eof_u(struct UFILE *f);

#endif
