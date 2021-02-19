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
#ifndef ___PANGOPRINt_H___
#define ___PANGOPRINt_H___

#include <pango/pangocairo.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include "utpdf.h"

typedef struct pango_cairo_print_object {
    cairo_surface_t *surface;
    cairo_t *cr;
    PangoFontDescription *desc;
    PangoLayout *layout;
    double phys_width, phys_height;
    double l_width, l_height;
    enum direction axis;
} pcobj; 

extern pcobj *pcobj_pdf_new
	(cairo_write_func_t write_func, int *out_fd,
         double width, double height);
extern pcobj *pcobj_ps_new
	(cairo_write_func_t write_func, int *out_fd,
         double width, double height);
extern void pcobj_free(pcobj *obj);
extern void pcobj_setfont(pcobj *obj, char *family, double size);
extern void pcobj_setsize(pcobj *obj, double size);
extern void pcobj_settext(pcobj *obj, const char *str);
extern void pcobj_print(pcobj *obj, const char *str);
extern void pcobj_weight(pcobj *obj, PangoWeight w);
extern void pcobj_style(pcobj *obj, PangoStyle style);
extern void pcobj_font_face(pcobj *obj, PangoStyle style, PangoWeight w);
extern double pcobj_font_ascent(pcobj *obj);
extern double pcobj_font_descent(pcobj *obj);
extern double pcobj_font_height(pcobj *obj);
extern double pcobj_width(pcobj *obj);
extern double pcobj_ink_width(pcobj *obj);
extern double pcobj_text_width(pcobj *obj, const char *str);
extern void pcobj_move_to(pcobj *obj, double x, double y);
extern void pcobj_draw_watermark(pcobj *obj, char *text, char *font,
                                 double x, double y, double dx, double dy,
                                 PangoWeight weight, PangoStyle style,
                                 double r, double g, double b);

extern void pcobj_upside_down(pcobj *obj);
extern void pcobj_turn_right(pcobj *obj);
extern void pcobj_turn_left(pcobj *obj);
extern void pcobj_setdir(pcobj *obj, enum direction d);

#endif
// end of pangoprint.h
