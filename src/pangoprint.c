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
    pango_layout_set_font_description(obj->layout, obj->desc);
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

PangoFontMetrics *pcobj_fontmetrics(pcobj *obj){
    PangoFontset *fontset;
    PangoFontMap *fontmap;
    PangoContext *context;
    PangoFontMetrics *metrics;
    
    fontmap=pango_cairo_font_map_get_default();
    context=pango_font_map_create_context(fontmap);
    fontset=pango_font_map_load_fontset(fontmap, context, obj->desc, pango_language_get_default());
    metrics=pango_fontset_get_metrics (fontset);
    // g_object_unref(context);
    return metrics;
}

double pcobj_font_ascent(pcobj *obj){
    PangoFontMetrics *metrics=pcobj_fontmetrics(obj);
    return (double)pango_font_metrics_get_ascent(metrics)/PANGO_SCALE;
}

double pcobj_font_descent(pcobj *obj){
    PangoFontMetrics *metrics=pcobj_fontmetrics(obj);
    return (double)pango_font_metrics_get_descent(metrics)/PANGO_SCALE;
}

double pcobj_font_height(pcobj *obj){
    PangoFontMetrics *metrics=pcobj_fontmetrics(obj);
    return (double)pango_font_metrics_get_height(metrics)/PANGO_SCALE;
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
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>

#define A4_w 595.27
#define A4_h 841.89

double cairo_text_width(cairo_t *cr, const char *str){
    cairo_text_extents_t t_ext;
    
    cairo_text_extents(cr, str, &t_ext);
    return t_ext.x_advance; // t_ext.width
}

#define C_RED    1, 0, 0    // red
#define C_BLUE   0, 0, 1    // blue
#define C_BLACK  0, 0, 0    // black
#define C_WHITE  1, 1, 1    // white
#define FONT_SIZE 16
#define LINE_HEIGHT (FONT_SIZE+4)
#define TOP 48
#define LEFT 72
#define LFONT_SIZE 36
#define LLINE_HEIGHT (LFONT_SIZE+4)

static void draw_text (cairo_t *cr, char *font){
    char buf[256];
    pcobj *obj;
    double width;
    int w;
    
    obj = pcobj_new(cr);

    // print fontname
    cairo_set_source_rgb(cr, C_BLACK);
    pcobj_setfont(obj, font, FONT_SIZE);
    pcobj_move_to(obj, LEFT, TOP);
    pcobj_print(obj, font);

    width=pcobj_text_width(obj, " normal ");
        
    for(w=1; w<=10; w++){
        // weight 100-1000
        double h=TOP+w*LINE_HEIGHT;
        double body_width;
        
        cairo_set_source_rgb (cr, C_BLACK);
        pcobj_move_to(obj, LEFT, h);
        pcobj_weight(obj, w*100);

        switch (w){
        case 3:
            pcobj_print(obj, "light");
            break;
        case 4:
            pcobj_print(obj, "normal");
            break;
        case 7:
            pcobj_print(obj, "bold");
            break;
        }
            
        snprintf(buf, 256, "weight %04d: 漢字／The quick brown fox jumped over the lazy dog.", w*100);
        
        pcobj_move_to (obj, LEFT+width, h);
        pcobj_print(obj, buf);

        body_width=pcobj_text_width(obj, buf);
        cairo_set_line_width(cr, 0.2);
        cairo_set_source_rgb (cr, C_RED);

        cairo_move_to(cr, LEFT, h);
        cairo_rel_line_to(cr, width+body_width, 0);
        cairo_stroke(cr);
    } // for(w=1; w<=10; w++)

    pcobj_weight(obj, PANGO_WEIGHT_NORMAL);
    cairo_set_source_rgb (cr, C_BLACK);

    // Italic
    pcobj_move_to(obj, LEFT, 12*LINE_HEIGHT+TOP);
    pcobj_style(obj, PANGO_STYLE_ITALIC);
    pcobj_print(obj, "Style italic: 漢字／The quick brown fox jumped over the lazy dog.");

    // Oblique
    pcobj_move_to(obj, LEFT, 13*LINE_HEIGHT+TOP);
    pcobj_style(obj, PANGO_STYLE_OBLIQUE);
    pcobj_print(obj, "Style oblique: 漢字／The quick brown fox jumped over the lazy dog.");

    pcobj_style(obj, PANGO_STYLE_NORMAL);
    {
        double h = 16*LINE_HEIGHT+TOP;
        double w;
        int i;
        
        pcobj_setsize(obj, LFONT_SIZE);
        pcobj_move_to(obj, LEFT, h);
        pcobj_print(obj, "MWil MWil");
        pcobj_move_to(obj, LEFT, h+LFONT_SIZE);
        pcobj_print(obj, "0123456789");
        pcobj_move_to(obj, LEFT, h+LFONT_SIZE*2);
        pcobj_print(obj, "日本語");
        w=pcobj_text_width(obj, "日");

        cairo_set_line_width(cr, 0.2);
        cairo_set_source_rgb (cr, C_BLUE);

        for (i=0; i<5; i++){
            cairo_move_to(cr, LEFT+w*i, h-pcobj_font_ascent(obj));
            cairo_rel_line_to(cr, 0, LFONT_SIZE*3);
        }
        cairo_stroke(cr);
        cairo_set_source_rgb (cr, C_BLACK);
    }
    cairo_show_page(cr);
    pcobj_free(obj);
}

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


int main(int argc, char **argv){
  cairo_t *cr;
  int i;
  // cairo_status_t status;
  cairo_surface_t *surface;
  int fd = openfd("p.pdf", O_CREAT|O_RDWR|O_TRUNC);

  setlocale(LC_ALL, "");
  
  surface = cairo_pdf_surface_create_for_stream
      ((cairo_write_func_t )write_func, (void *)&fd, A4_h, A4_w);
  cr = cairo_create(surface);
  cairo_set_source_rgb (cr, C_BLACK);

  for (i=1; i<argc; i++){
      draw_text (cr, argv[i]);
  }
  cairo_destroy (cr);
  cairo_surface_destroy(surface);
  
  return 0;
}

#endif

// end of pangoprint.c
