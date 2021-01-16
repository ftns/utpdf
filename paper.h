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

//
// paper sizes

#ifndef __PAPER_H__
#define __PAPER_H__

enum papers { a3, a4, a5, b3, b4, b5, letter, legal, PAPERS_END};

#define PNAME_DEFAULT a4 // enum papers

#define PNAME_SIZE 16
struct psize {
    char pname[PNAME_SIZE];
    double w;
    double h;
};

extern struct psize paper_sizes[PAPERS_END];
extern char *paper_default();


#endif
// end of paper.h
