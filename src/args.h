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
#ifndef __ARGS_H__
#define __ARGS_H__

#include "utpdf.h"

#define PDF_CONF_FILE ".utpdfrc"
#define PS_CONF_FILE ".utpsrc"


typedef struct arguments {
    // option flags
    int twocols, numbering, header, punchmark, duplex, portrait, longedge;
    int tab, notebook, fold_arrow, border, current_t, one_output, inch;
    int hfont_slant, hfont_weight, bfont_slant, bfont_weight;
    int side_slant, side_weight, wmark_slant, wmark_weight;
    int rotate_right, upside_down_page, force_duplex;
    // option strings
    char *fontname, *headerfont, *in_fname, *date_format, *headertext, *outfile;
    char *binded_edge, *paper, *wmark_text, *wmark_font;    
    // option length
    double fontsize, header_height, head_size, side_size;
    double wmark_r, wmark_g, wmark_b;
    // paper size and margins
    double pwidth, pheight, phys_width, phys_height;
    double binding, pleft, pright, ptop, pbottom, divide, betweenline;
    // file modified time
    time_t *mtime;
} args_t;

extern args_t *args;
extern args_t args_store;

extern void getargs(int argc, char **argv);

#endif
// end of args.h
