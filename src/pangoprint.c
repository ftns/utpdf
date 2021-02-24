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

#include "pangoprint.h"
#include <math.h>

pcobj *pcobj_setup(pcobj *obj, double width, double height){
    obj->cr = cairo_create(obj->surface);
    obj->desc = pango_font_description_new();    
    obj->layout = pango_cairo_create_layout (obj->cr);

    obj->phys_width = width;
    obj->phys_height = height;
    obj->l_width = width;
    obj->l_height = height;
    obj->axis = d_up;
    return obj;
}

pcobj *pcobj_pdf_new(cairo_write_func_t write_func, int *out_fd,
                     double width, double height){
    pcobj *obj = malloc(sizeof(pcobj));    
    obj->surface = cairo_pdf_surface_create_for_stream
        ((cairo_write_func_t )write_func, (void *)out_fd,
         width, height);
    return pcobj_setup(obj, width, height);
}

pcobj *pcobj_ps_new(cairo_write_func_t write_func, int *out_fd,
                    double width, double height){
    pcobj *obj = malloc(sizeof(pcobj));    
    obj->surface = cairo_ps_surface_create_for_stream
        ((cairo_write_func_t )write_func, (void *)out_fd,
         width, height);
    return pcobj_setup(obj, width, height);
}

void pcobj_free(pcobj *obj){
    pango_font_description_free(obj->desc);
    g_object_unref(obj->layout);
    cairo_destroy(obj->cr);
    cairo_surface_destroy(obj->surface);
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
    // pango_cairo_update_layout (obj->cr, obj->layout);
}    

void pcobj_print(pcobj *obj, const char *str){
    pcobj_settext(obj, str);
    pango_cairo_show_layout (obj->cr, obj->layout);
}

/*
  enum PangoWeight: 100-1000
    PANGO_WEIGHT_LIGHT   300
    PANGO_WEIGHT_NORMAL  400
    PANGO_WEIGHT_BOLD    700
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
    PANGO_STYLE_ITALIC   the font is slanted in an italic style.
    PANGO_STYLE_OBLIQUE  the font is slanted, but in a roman style.
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
    pcobj_settext(obj, str);
    return pcobj_width(obj);
}

double pcobj_width(pcobj *obj){
    PangoRectangle ink, logical;

    pango_layout_get_extents(obj->layout, &ink, &logical);
    return logical.width/PANGO_SCALE;
}

double pcobj_ink_width(pcobj *obj){
    PangoRectangle ink, logical;

    pango_layout_get_extents(obj->layout, &ink, &logical);
    return ink.width/PANGO_SCALE;
}

void pcobj_move_to(pcobj *obj, double x, double y){
    cairo_move_to(obj->cr, x, y-pcobj_font_ascent(obj));
}

#define SAMPLE_SIZE 64

void pcobj_draw_watermark(pcobj *obj, char *text, char *font,
                          double x, double y, double dx, double dy,
                          PangoWeight weight, PangoStyle style,
                          double r, double g, double b){
    double rad, tan_t, h, w, k, new_h, new_w, e, new_size;

    pcobj_setfont(obj, font, SAMPLE_SIZE);
    pcobj_font_face(obj, style, weight);
    cairo_set_source_rgb(obj->cr, r, g, b);

    cairo_save(obj->cr);{
        cairo_translate(obj->cr, x+dx/2, y+dy/2);
        pango_cairo_update_layout (obj->cr, obj->layout);

        w=pcobj_text_width(obj, text);
        h=pcobj_font_height(obj);
        // h=pcobj_font_ascent(obj);
        k=h/w;
        tan_t=dy/dx;
        rad=atan(tan_t);
        r=sqrt(pow(dx,2)+pow(dy,2));

        if (dx>dy){
            e=r/(2*(1+(tan_t/k)));
        } else {
            e=r/(2*(1+1/(k*tan_t)));
        }
        new_w=r-2*e;
        new_h=k*new_w;
        new_size=1.1*SAMPLE_SIZE*new_w/w;
        pcobj_setsize(obj, new_size);
        pcobj_settext(obj, text);
        new_w = pcobj_ink_width(obj);
        new_h = pcobj_font_height(obj);

        cairo_rotate(obj->cr, -rad);
        pango_cairo_update_layout (obj->cr, obj->layout);

        cairo_move_to(obj->cr, -new_w/2, -new_h/2);
        pango_cairo_show_layout(obj->cr, obj->layout);
        cairo_set_source_rgb(obj->cr, 0, 0, 0); // C_BLACK
    } cairo_restore(obj->cr);
    
#ifdef SINGLE_DEBUG
    cairo_rectangle(obj->cr, x, y, dx, dy);
    cairo_move_to(obj->cr, x, y+dy);
    cairo_line_to(obj->cr, x+dx, y);
    cairo_stroke(obj->cr);
#endif
    pango_cairo_update_layout (obj->cr, obj->layout);
}

void dump_matrix(pcobj *obj){
    cairo_matrix_t mat;
    cairo_get_matrix(obj->cr, &mat);
    fprintf(stderr, "/ %- 3.2f %- 3.2f \\  / %- 7.2f \\\n", mat.xx, mat.xy, mat.x0);
    fprintf(stderr, "\\ %- 3.2f %- 3.2f /  \\ %- 7.2f /\n", mat.yx, mat.yy, mat.y0);
}

char *dir2str(enum direction d){
    switch(d){
    case d_none:
        return "d_none";
    case d_up:
        return "d_up";
    case d_down:
        return "d_down";
    case d_left:
        return "d_left";
    case d_right:
        return "d_right";
    }
}

void pcobj_upside_down(pcobj *obj){
#if defined(SINGLE_DEBUG)
    fprintf(stderr, "upside_down(): %s -> %s\n", dir2str(obj->axis), dir2str(-obj->axis));
#endif
    pcobj_setdir(obj, -obj->axis);
}

void pcobj_turn_right(pcobj *obj){
    enum direction new_axis;
    
    switch(obj->axis){
    case d_none:
        new_axis=d_none;
        break;
    case d_up:
        new_axis=d_right;
        break;
    case d_down:
        new_axis=d_left;
        break;
    case d_right:
        new_axis=d_down;
        break;
    case d_left:
        new_axis=d_up;
        break;
    }
#if defined(SINGLE_DEBUG)
    fprintf(stderr, "turn_right(): %s -> %s\n", dir2str(obj->axis), dir2str(new_axis));
#endif
    pcobj_setdir(obj, new_axis);
}

void pcobj_turn_left(pcobj *obj){
    enum direction new_axis;
    
    switch(obj->axis){
    case d_none:
        new_axis=d_none;
        break;
    case d_up:
        new_axis=d_left;
        break;
    case d_down:
        new_axis=d_right;
        break;
    case d_right:
        new_axis=d_up;
        break;
    case d_left:
        new_axis=d_down;
        break;
    }
#if defined(SINGLE_DEBUG)
    fprintf(stderr, "turn_left(): %s -> %s\n", dir2str(obj->axis), dir2str(new_axis));
#endif
    pcobj_setdir(obj, new_axis);
}

void pcobj_setdir(pcobj *obj, enum direction d){
    cairo_matrix_t mat;

    obj->axis=d;
    switch (d){
    case d_none:
    case d_up:
        obj->l_width=obj->phys_width;
        obj->l_height=obj->phys_height;
        mat.xx=1; mat.xy=0; mat.x0=0;
        mat.yx=0; mat.yy=1; mat.y0=0;
        break;
    case d_down:
        obj->l_width=obj->phys_width;
        obj->l_height=obj->phys_height;
        mat.xx=-1; mat.xy= 0; mat.x0= obj->phys_width;
        mat.yx= 0; mat.yy=-1; mat.y0= obj->phys_height;
        break;
    case d_right:
        obj->l_width=obj->phys_height;
        obj->l_height=obj->phys_width;
        mat.xx= 0; mat.xy=-1; mat.x0= obj->phys_width;
        mat.yx= 1; mat.yy= 0; mat.y0= 0;
        break;
    case d_left:
        obj->l_width=obj->phys_height;
        obj->l_height=obj->phys_width;
        mat.xx=  0; mat.xy= 1; mat.x0= 0;
        mat.yx= -1; mat.yy= 0; mat.y0= obj->phys_height;
        break;
    }
    cairo_set_matrix(obj->cr, &mat);
    pango_cairo_update_layout(obj->cr, obj->layout);
}

/* --- debug part --- */

#ifdef SINGLE_DEBUG

#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include "io.h"

#define A4_w 595.27
#define A4_h 841.89

#define PS_TEST 1

double cairo_text_width(cairo_t *cr, const char *str){
    cairo_text_extents_t t_ext;
    
    cairo_text_extents(cr, str, &t_ext);
    return t_ext.x_advance; // t_ext.width
}

#define C_RED    1, 0, 0    // red
#define C_BLUE   0, 0, 1    // blue
#define C_BLACK  0, 0, 0    // black
#define C_WHITE  1, 1, 1    // white
#define C_WATERMARK 0.9, 0.9, 1 // watermark
#define C_RECT  0.5, 1, 0.5
#define FONT_SIZE 16
#define LINE_HEIGHT (FONT_SIZE+4)
#define TOP  72
#define LEFT 72
#define LFONT_SIZE 36
#define LLINE_HEIGHT (LFONT_SIZE+4)

static void draw_page (pcobj *obj, char *font){
    char buf[S_LEN];
    double width;
    int w;

    
    // print watermark
    pcobj_draw_watermark(obj, "The quick fox ", "serif",
                         LEFT, TOP, A4_h-LEFT*2, A4_w-TOP*2,
                         PANGO_WEIGHT_BOLD, PANGO_STYLE_ITALIC,
                         C_WATERMARK);

    fprintf(stderr, "After pcobj_draw_watermark()\n");
    dump_matrix(obj);
    
    // print fontname
    cairo_set_source_rgb(obj->cr, C_BLACK);
    pcobj_setfont(obj, font, FONT_SIZE);
    pcobj_font_face(obj, PANGO_STYLE_NORMAL, PANGO_WEIGHT_NORMAL);
    pcobj_move_to(obj, LEFT, TOP);
    pcobj_print(obj, font);

    cairo_set_source_rgb(obj->cr, C_RECT);    
    cairo_set_line_width(obj->cr, 3);
    cairo_rectangle(obj->cr, 0, 0, LEFT, TOP);
    cairo_stroke(obj->cr);
    cairo_set_source_rgb(obj->cr, C_BLACK);
    
    // upside-down
    pcobj_upside_down(obj);
    
    fprintf(stderr, "upside-down \n");
    // dump_matrix(obj);

    pcobj_style(obj, PANGO_STYLE_ITALIC);
    pcobj_move_to(obj, LEFT, TOP);
    pcobj_print(obj, font);
    pcobj_move_to(obj, LEFT, TOP+LINE_HEIGHT);
    pcobj_print(obj, "upside down");

    cairo_set_source_rgb(obj->cr, C_RECT);    
    cairo_rectangle(obj->cr, 1, 1, LEFT, TOP);
    cairo_stroke(obj->cr);
    cairo_set_source_rgb(obj->cr, C_BLACK);
    pcobj_upside_down(obj);

    // turn right
    pcobj_turn_right(obj);
    pcobj_move_to(obj, TOP, LEFT);
    pcobj_print(obj, "turn right");

    cairo_set_source_rgb(obj->cr, C_RECT);    
    cairo_rectangle(obj->cr, 1, 1, TOP, LEFT);
    cairo_stroke(obj->cr);
    cairo_set_source_rgb(obj->cr, C_BLACK);

    // turn left
    pcobj_turn_left(obj);
    pcobj_turn_left(obj);
    pcobj_move_to(obj, TOP, LEFT);
    pcobj_print(obj, "turn left");

    cairo_set_source_rgb(obj->cr, C_RECT);    
    cairo_rectangle(obj->cr, 1, 1, TOP, LEFT);
    cairo_stroke(obj->cr);
    cairo_set_source_rgb(obj->cr, C_BLACK);

    // print every weight
    pcobj_style(obj, PANGO_STYLE_NORMAL);
    pcobj_turn_right(obj);
    width=pcobj_text_width(obj, " normal ");
        
    for(w=1; w<=10; w++){
        // weight 100-1000
        double h=TOP+w*LINE_HEIGHT;
        double body_width;
        
        cairo_set_source_rgb (obj->cr, C_BLACK);
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
            
        snprintf(buf, S_LEN, "weight %04d: 漢字／The quick brown fox jumped over the lazy dog.", w*100);
        
        pcobj_move_to (obj, LEFT+width, h);
        pcobj_print(obj, buf);

        body_width=pcobj_text_width(obj, buf);
        cairo_set_line_width(obj->cr, 0.2);
        cairo_set_source_rgb (obj->cr, C_RED);

        cairo_move_to(obj->cr, LEFT, h);
        cairo_rel_line_to(obj->cr, width+body_width, 0);
        cairo_stroke(obj->cr);
    } // for(w=1; w<=10; w++)

    pcobj_weight(obj, PANGO_WEIGHT_NORMAL);
    cairo_set_source_rgb (obj->cr, C_BLACK);

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

        cairo_set_line_width(obj->cr, 0.2);
        cairo_set_source_rgb (obj->cr, C_BLUE);

        for (i=0; i<5; i++){
            cairo_move_to(obj->cr, LEFT+w*i, h-pcobj_font_ascent(obj));
            cairo_rel_line_to(obj->cr, 0, LFONT_SIZE*3);
        }
        cairo_stroke(obj->cr);
        cairo_set_source_rgb(obj->cr, C_BLACK);
    }
    cairo_show_page(obj->cr);
    pcobj_upside_down(obj);
}


int main(int argc, char **argv){
    int i, fd;
    pcobj *obj;
    
    if (argc <= 1){
        fprintf(stderr, "No font specified.\n");
        exit(1);
    }
    setlocale(LC_ALL, "");

#if PS_TEST
    // PostScript
    fd = STDOUT_FILENO;
    obj = pcobj_ps_new((cairo_write_func_t ) write_ps_duplex, &fd, A4_w, A4_h);
    // cairo_ps_surface_restrict_to_level(obj->surface, CAIRO_PS_LEVEL_2);
    cairo_ps_surface_dsc_comment
        (obj->surface, "%%Requirements: duplex");
    cairo_ps_surface_dsc_begin_setup(obj->surface);
    cairo_ps_surface_dsc_comment
        (obj->surface,
         "%%IncludeFeature: *Duplex DuplexNoTumble");
    // set orientation
    cairo_ps_surface_dsc_begin_page_setup (obj->surface);
    cairo_ps_surface_dsc_comment
        (obj->surface, "%%PageOrientation: Landscape");

    // cairo_identity_matrix(obj->cr);
    // dump_matrix(obj);
#else
    // PDF
    fd = openfd("p.pdf", O_CREAT|O_RDWR|O_TRUNC);
    obj = pcobj_pdf_new((cairo_write_func_t ) write_func, &fd, A4_h, A4_w);
#endif
  
    cairo_set_source_rgb (obj->cr, C_BLACK);

    fprintf(stderr, "Initial state\n");
    dump_matrix(obj);

#if PS_TEST
    // pcobj_turn_right(obj);
    pcobj_setdir(obj, d_right);
    fprintf(stderr, "After setdir(d_right)\n");
    dump_matrix(obj);
#endif

    for (i=1; i<argc; i++){
        draw_page(obj, argv[i]);
    }
    pcobj_free(obj);
    close(fd);
  
    return 0;
}

#endif

// end of pangoprint.c
