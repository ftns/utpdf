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

#ifndef __DRAWING_H__
#define __DRAWING_H__

#include "utpdf.h"
#include "io.h"
#include "args.h"
#include "coord.h"
#include "pangoprint.h"

extern void show_text_at_center(pcobj *obj, const char *str);
extern void show_text_at_right(pcobj *obj, const char *str);
extern void show_text_at_left(pcobj *obj, const char *str);
extern void draw_rel_line
    (cairo_t *cr, double x, double y, double dx, double dy, double line_w,
     double r, double g, double b);

extern void draw_mark(cairo_t *cr, enum direction d, double x, double y);
extern void draw_return_arrow
    (cairo_t *cr, double x, double y, double edge, double width,
     double r, double g, double b);
extern void draw_cont_arrow
    (cairo_t *cr, double x, double y, double edge, double width,
     double r, double g, double b);

extern void draw_header
   (pcobj *obj, args_t *args, int page, mcoord_t *mcoord,
    scoord_t *scoord, char *datebuf);
extern void draw_lines
    (pcobj *obj, UFILE *in_f, args_t *args,int lineperpage,
    int *fline,  mcoord_t *mcoord, scoord_t *scoord);
extern void draw_file(pcobj *obj, UFILE *in_f, args_t *args, int last_file);

#endif

// end of drawing.h

