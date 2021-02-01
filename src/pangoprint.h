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
#ifndef ___PANGOPRINt_H___
#define ___PANGOPRINt_H___

#include <pango/pangocairo.h>
#include <cairo.h>

typedef struct pango_cairo_print_object {
    cairo_t *cr;
    PangoFontDescription *desc;
    PangoLayout *layout;
    // PangoFontMetrics *metrics;
    // PangoFontset *fontset;
    // PangoFontMap *fontmap;
    // PangoContext *context;
} pcobj; 

extern pcobj *pcobj_new(cairo_t *cr);
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
extern double pcobj_text_width(pcobj *obj, const char *str);
extern void pcobj_move_to(pcobj *obj, double x, double y);

#endif
// end of pangoprint.h
