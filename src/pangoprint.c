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

#include "pangoprint.h"

pcobj *pcobj_new(cairo_t *cr){    
    pcobj *obj = malloc(sizeof(pcobj));
    obj->cr = cr;
    obj->desc = pango_font_description_new();
    obj->layout = pango_cairo_create_layout (cr);
    return obj;
}

void pcobj_free(pcobj *obj){
    pango_font_description_free(obj->desc);
    g_object_unref(obj->layout);
    free(obj);
}

void pcobj_setfont(pcobj *obj, char *family, double size){
    pango_font_description_set_family(obj->desc, family);
    pango_font_description_set_absolute_size(obj->desc, size*PANGO_SCALE);
    pango_layout_set_font_description (obj->layout, obj->desc);
}

void pcobj_setsize(pcobj *obj, double size){
    pango_font_description_set_absolute_size(obj->desc, size*PANGO_SCALE);
    pango_layout_set_font_description (obj->layout, obj->desc);
}

void pcobj_settext(pcobj *obj, const char *str){
    pango_layout_set_text (obj->layout, str, -1);
    pango_cairo_update_layout (obj->cr, obj->layout);
}    

void pcobj_print(pcobj *obj, const char *str){
    pcobj_settext(obj, str);
    pango_cairo_show_layout (obj->cr, obj->layout);
}

/*
  enum PangoWeight: 100-1000
    PANGO_WEIGHT_LIGHT  300
    PANGO_WEIGHT_NORMAL 400
    PANGO_WEIGHT_BOLD   700
*/
void pcobj_weight(pcobj *obj, PangoWeight w){
    if ((w<100)||(w>1000)){
        fprintf(stderr, "pcobj_weight: weight must be 100-1000, but %d\n", w);
    } else {        
        pango_font_description_set_weight(obj->desc, w);
        pango_layout_set_font_description (obj->layout, obj->desc);
    }
}

/*
  enum PangoStyle 
    PANGO_STYLE_NORMAL   the font is upright,
    PANGO_STYLE_OBLIQUE  the font is slanted, but in a roman style.
    PANGO_STYLE_ITALIC   the font is slanted in an italic style.
*/
void pcobj_style(pcobj *obj, PangoStyle style){
    pango_font_description_set_style(obj->desc, style);
    pango_layout_set_font_description (obj->layout, obj->desc);
}

void pcobj_font_face(pcobj *obj, PangoStyle style, PangoWeight w){
    if ((w<100)||(w>1000)){
        fprintf(stderr, "pcobj_font_face: weight must be 100-1000, but %d\n", w);
    } else {        
        pango_font_description_set_style(obj->desc, style);
        pango_font_description_set_weight(obj->desc, w);
        pango_layout_set_font_description (obj->layout, obj->desc);
    }
}

double pcobj_font_ascent(pcobj *obj){
    int base;
    PangoRectangle ink, logical;
        
    pango_cairo_update_layout (obj->cr, obj->layout);
    base=pango_layout_get_baseline(obj->layout);
    pango_layout_get_extents(obj->layout, &ink, &logical);
    return (base-logical.y)/PANGO_SCALE;
}

double pcobj_font_descent(pcobj *obj){
    int base;
    PangoRectangle ink, logical;
        
    pango_cairo_update_layout (obj->cr, obj->layout);
    base=pango_layout_get_baseline(obj->layout);
    pango_layout_get_extents(obj->layout, &ink, &logical);
    return (logical.y+logical.height-base)/PANGO_SCALE;
}

double pcobj_font_height(pcobj *obj){
    int base;
    PangoRectangle ink, logical;
        
    base=pango_layout_get_baseline(obj->layout);
    pango_cairo_update_layout(obj->cr, obj->layout);
    pango_layout_get_extents(obj->layout, &ink, &logical);
    return logical.height/PANGO_SCALE;
}

double pcobj_text_width(pcobj *obj, const char *str){
    PangoRectangle ink, logical;
        
    // base=pango_layout_get_baseline(obj->layout);
    pcobj_settext(obj, str);
    pango_cairo_update_layout (obj->cr, obj->layout);
    pango_layout_get_extents(obj->layout, &ink, &logical);
    return logical.width/PANGO_SCALE;
}

void pcobj_move_to(pcobj *obj, double x, double y){
    cairo_move_to(obj->cr, x, y-pcobj_font_ascent(obj));
    //    cairo_move_to(obj->cr, x, y);
}

// --- debug part ---

#ifdef SINGLE_DEBUG

#include <cairo-pdf.h>
#define A4_w 595.27
#define A4_h 841.89

double cairo_text_width(cairo_t *cr, const char *str){
    cairo_text_extents_t t_ext;
    
    cairo_text_extents(cr, str, &t_ext);
    return t_ext.x_advance; // t_ext.width
}

#define CAIRO_STR "Cairo 漢字"
#define FONT "IPAMincho"
#define FONT_SIZE 16
#define X_AXIS 24.0
#define Y_AXIS 24.0
#define TEST_STR "Pango 漢字／The quick brown fox jumped over the lazy dog."
#define C_BLUE   0,   0,   1    // blue
#define C_BLACK  0,   0,   0    // black

int main(){
    pcobj *obj;
    PangoRectangle ink, logical;
    double baseline;
    double x, y, w, h;
    cairo_t *cr;
    cairo_surface_t *surface;
    cairo_text_extents_t extents;
    double c_width, ink_x, ink_w, a, d;
    
    surface = cairo_pdf_surface_create("pprint.pdf", A4_h, A4_w);
    cr = cairo_create(surface);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    // cairo_move_to(cr, 24, 24);

    cairo_set_font_size(cr, FONT_SIZE);
    cairo_select_font_face(cr, FONT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to(cr, 24, 24);
    cairo_show_text(cr, CAIRO_STR);
    cairo_text_extents(cr, TEST_STR, &extents);
    c_width=cairo_text_width(cr, CAIRO_STR);

    // pango
    obj=pcobj_new(cr);    
    pcobj_move_to(obj, c_width+X_AXIS, Y_AXIS);
    pcobj_setfont(obj, FONT, FONT_SIZE);

    cairo_set_source_rgb(cr, C_BLUE);
    pcobj_print(obj, TEST_STR);
    cairo_set_source_rgb(cr, C_BLACK);
    
    pango_layout_get_extents(obj->layout, &ink, &logical);
    cairo_get_current_point(cr, &x, &y);
    ink_x=x+ink.x/PANGO_SCALE;
    ink_w=ink.width/PANGO_SCALE;
    x+=logical.x/PANGO_SCALE;
    y+=logical.y/PANGO_SCALE;
    w=logical.width/PANGO_SCALE;
    h=logical.height/PANGO_SCALE;
    baseline=pango_layout_get_baseline(obj->layout)/PANGO_SCALE+y;
    a=pcobj_font_ascent(obj);
    d=pcobj_font_descent(obj);
    fprintf(stderr, "Coordinate x: %2.1f, y: %2.1f\n", c_width+X_AXIS, Y_AXIS);
    fprintf(stderr, "Pango x: %2.1f, y: %2.1f, w: %2.1f, h: %2.1f, base: %2.1f\n",
            x, y, w, h, baseline);
    fprintf(stderr, "  Ink x: %2.1f, w; %2.1f\n", ink_x, ink_w);
    fprintf(stderr, "  ascent: %2.1f, descent: %2.1f\n", a, d);
    fprintf(stderr, "Cairo x: %2.1f, y: %2.1f, w: %2.1f, h: %2.1f\n",
            extents.x_bearing, extents.y_bearing, extents.width, extents.height);
    cairo_set_line_width(cr, 0.3);
    cairo_rectangle(cr, 24, y, c_width, h);
    cairo_rectangle(cr, c_width+24, y, w, h);
    cairo_move_to(cr, 24, baseline);
    cairo_rel_line_to(cr, c_width+w, 0);
    cairo_stroke(cr);

    cairo_move_to(cr, 25, 100);
    pcobj_print(obj, "This is a ");
    w=pcobj_text_width(obj, "This is a ");
    cairo_rel_move_to(cr, w, 0);
    pcobj_print(obj, "test.");
    
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
}

#endif

// end of pangoprint.c
