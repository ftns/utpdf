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

#ifndef __DRAWING_H__
#define __DRAWING_H__

#include "utpdf.h"
#include "io.h"
#include "args.h"
#include "coord.h"

typedef struct sub_coordinates {
	double text_left, num_right, body_inset; 
	double body_top, oneline_h;
} scoord_t;

extern double font_ascent(cairo_t *cr);
extern double font_descent(cairo_t *cr);

extern double text_width(cairo_t *cr, const char *str);
extern void show_text_at_center(cairo_t *cr, const char *str);
extern void show_text_at_right(cairo_t *cr, const char *str);
extern void show_text_at_left(cairo_t *cr, const char *str);
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
   (cairo_t *cr, args_t *arg, int page, mcoord_t *mcoord,
    scoord_t *scoord, char *datebuf);
extern void draw_limited_text
    (cairo_t *cr, UFILE *in_f, int tab, const double limit, int *cont);
extern void draw_lines
    (cairo_t *cr, UFILE *in_f, args_t *arg,int lineperpage,
     mcoord_t *mcoord, scoord_t *scoord);
extern int draw_pages(cairo_t *cr, UFILE *in_f, args_t *arg);

#endif

// end of drawing.h

