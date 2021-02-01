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

#include <math.h>
#include <locale.h>
#include "coord.h"
#include "drawing.h"
#include "utpdf.h"
#include "args.h"
#include "pangoprint.h"

#define PI 3.14159265359


void show_text_at_center(pcobj *obj, const char *str){
    cairo_rel_move_to(obj->cr, -pcobj_text_width(obj, str)/2, 0);
    pcobj_print(obj, str);
}

void show_text_at_right(pcobj *obj, const char *str){
    cairo_rel_move_to(obj->cr, -pcobj_text_width(obj, str), 0);
    pcobj_print(obj, str);
}

void show_text_at_left(pcobj *obj, const char *str){
    pcobj_print(obj, str);
}

void draw_rel_line(cairo_t *cr, double x, double y, double dx, double dy,
		   double line_w, double r, double g, double b){
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, line_w);
    cairo_move_to(cr, x, y);
    cairo_rel_line_to(cr, dx, dy);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, C_BLACK);
}

void draw_rectangle(cairo_t *cr, double x, double y, double dx, double dy,
                    double line_w, double r, double g, double b){
    cairo_set_source_rgb(cr, r, g, b);
    cairo_set_line_width(cr, line_w);
    cairo_rectangle(cr, x, y, dx, dy);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, C_BLACK);
}

void draw_mark(cairo_t *cr, enum direction d, double x, double y){
    cairo_move_to(cr, x, y);
    switch(d){
    case d_up:
	cairo_rel_move_to(cr, 0, -MARK_H/2);
	cairo_rel_line_to(cr, -MARK_W/2, MARK_H);
	cairo_rel_line_to(cr, MARK_W, 0);
	cairo_close_path(cr);
	cairo_fill(cr);
	break;
    case d_down:
	cairo_rel_move_to(cr, 0, MARK_H/2);
	cairo_rel_line_to(cr, -MARK_W/2, -MARK_H);
	cairo_rel_line_to(cr, MARK_W, 0);
	cairo_close_path(cr);
	cairo_fill(cr);
	break;
    case d_right:
	cairo_rel_move_to(cr, MARK_H/2, 0);
	cairo_rel_line_to(cr, -MARK_H, -MARK_W/2);
	cairo_rel_line_to(cr, 0, MARK_W);
	cairo_close_path(cr);
	cairo_fill(cr);
	break;
    case d_left:
	cairo_rel_move_to(cr, -MARK_H/2, 0);
	cairo_rel_line_to(cr, MARK_H, -MARK_W/2);
	cairo_rel_line_to(cr, 0, MARK_W);
	cairo_close_path(cr);
	cairo_fill(cr);
	break;
    default:
	break;
	// do nothing
    }
}


void draw_return_arrow(cairo_t *cr, double x, double y, double edge, double width,
		       double r, double g, double b){
    double radius=edge/2;

    cairo_save(cr);
    cairo_translate(cr, x+radius, y+radius);
    {
	cairo_set_source_rgb(cr, r, g, b);
	cairo_set_line_width(cr, width);

	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, 0, radius);
	cairo_stroke(cr);

	cairo_move_to(cr, radius, radius-width/2);
	cairo_line_to(cr, 0,      radius-width/2);
	cairo_stroke(cr);
	
	cairo_arc(cr, 0, 0, radius-width/2, PI*3/2, PI/2);
	cairo_stroke(cr);
    }
    cairo_restore(cr);
}


void draw_cont_arrow(cairo_t *cr, double x, double y, double edge, double width,
		     double r, double g, double b){
    double radius=edge/2;

    cairo_save(cr);
    cairo_translate(cr, x+radius, y+radius);
    {
	cairo_move_to(cr, 0, 0);
	cairo_set_source_rgb(cr, r, g, b);
	cairo_set_line_width(cr, width);

	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, 0, radius);
	cairo_stroke(cr);

	cairo_move_to(cr, -radius, radius-width/2);
	cairo_line_to(cr, 0, radius-width/2);
	cairo_stroke(cr);

	cairo_arc(cr, 0, 0, radius-width/2, PI/2, PI*3/2); // PI/2, PI*7/4); 
	cairo_stroke(cr);
    }
    cairo_restore(cr);
}

#define PAGEBUFLEN 64

void draw_header(pcobj *obj, args_t *args, int page, mcoord_t *mcoord,
                 scoord_t *scoord, char *datebuf){
    double hbaseline, hinset, gap;
    char pagebuf[PAGEBUFLEN];
    cairo_t *cr=obj->cr;
    double ascent, descent;
    
    // hbaseline: the baseline of header font
    // cairo_select_font_face (cr, , args->hfont_slant, args->hfont_weight);
    pcobj_setfont(obj, args->headerfont, args->hfont_large);
    pcobj_font_face(obj, args->hfont_slant, args->hfont_weight);
    ascent=pcobj_font_ascent(obj);
    descent=pcobj_font_descent(obj);
    gap = (scoord->body_top - mcoord->head_top - pcobj_font_height(obj))/3;
    hbaseline = scoord->body_top - gap - descent;

    // hinset: header inset
    cairo_set_font_size (cr, args->hfont_medium);
    hinset = pcobj_text_width(obj, "0");
		
    // draw left part: modified date
    pcobj_setfont(obj, args->headerfont, args->hfont_medium);
    pcobj_weight(obj, args->hfont_weight);
    pcobj_style(obj, args->hfont_slant);
    pcobj_move_to(obj, mcoord->body_left+hinset, hbaseline);
    show_text_at_left(obj, datebuf);
  
    // center part: filename
    pcobj_setsize(obj, args->hfont_large);
    pcobj_move_to(obj, mcoord->body_left+mcoord->bwidth/2, hbaseline);

    if (args->headertext == NULL) {
	show_text_at_center(obj, args->in_fname);
    } else {
	show_text_at_center(obj, args->headertext);
    }
    // right part: page
    snprintf(pagebuf, PAGEBUFLEN, "page: %d  ", page);
    pcobj_setsize(obj, args->hfont_medium);
    pcobj_move_to(obj, mcoord->body_left + mcoord->bwidth - hinset, hbaseline);
    show_text_at_right(obj, pagebuf);

    // settle font face default
    pcobj_font_face(obj, PANGO_STYLE_NORMAL, PANGO_WEIGHT_NORMAL);
}


void draw_limited_text(pcobj *obj, UFILE *in_f, int tab, const double limit,
                       int *cont, double orig_left, double baseline){
    int clen, olen;
    double em, tabw; // width of "M", tab
    double cur_left=orig_left, limit_x;
    static char outbuf[BUFLEN], rbuf[8];
    static int over_sp=0;
    
    // outbuf=""
    outbuf[0] = '\0';
    olen = 0;

    // cairo_get_current_point (cr, &cur_left, &cur_base);
    limit_x = limit + orig_left;

    em=pcobj_text_width(obj, "M");
    tabw=em*tab;
    cur_left+=em*over_sp;
    over_sp=0;
    pcobj_move_to(obj, cur_left, baseline);
    
    while((cur_left+pcobj_text_width(obj, outbuf)<limit_x)
          && (get_one_uchar(in_f, rbuf)>0)
          && (rbuf[0] != 0x0D) && (rbuf[0] != 0x0A)){
	if (rbuf[0] == '\t') {
            // tab
	    double cur_right = cur_left+pcobj_text_width(obj, outbuf);
	    double new_right = tabw*ceil((cur_right+em-orig_left)/tabw)+orig_left;
	    if (new_right < limit_x) {
		// tab jump
		pcobj_print(obj, outbuf);
		cur_left = new_right;
		pcobj_move_to(obj, cur_left, baseline);
		outbuf[0] = '\0';
		olen = 0;
	    } else {
		// tab jump -> overflow
		over_sp = ceil((new_right-limit_x)/em); // over_sp is static.
		*cont=1;
		pcobj_print(obj, outbuf);
		return;
	    }
	} else if (rbuf[0] == 0x0D){
	    // Is end of line is "CR" or "CRLF"?
	    char nextbuf[8];
            int clen = get_one_uchar(in_f, nextbuf);
            *cont = 0;
            if ((clen == 0)||(nextbuf[0]==0x0A)) {
                break;
            } else {
                rewind_u(in_f, clen);
                break;
            }
        } else if (rbuf[0] == 0x0A){
            *cont = 0;
            break;
	} else {	
	    // outbuf += cur.firstchar();
	    // cur.step_one_char();
	    // Remark: On UTF-8, 1char is 1-6bytes(variable length)
	    int i;
	    clen = nbytechar(rbuf[0]);
	    for (i=0; i<clen; i++) {
	    	outbuf[olen++] = rbuf[i];
	    }
	    outbuf[olen]='\0';
	}
    } // while
    
    if (cur_left+pcobj_text_width(obj, outbuf)>limit_x){
	// overflow
	*cont = 1;
	// remove 1char from outbuf, and print it
	outbuf[olen-clen] = '\0';
	pcobj_print(obj, outbuf);
	// push back 1char.
	rewind_u(in_f, clen);
	return; 
    } else {
	// finish this line
	pcobj_print(obj, outbuf);
	*cont = 0;
	return;
    }    
}


void draw_lines(pcobj *obj, UFILE *in_f, args_t *args, int lineperpage,
		mcoord_t *mcoord, scoord_t *scoord){
    int pline=1; 	// line number of this page
    static int fline=1; // line nunber of this file
    double limitw, baseline;
    cairo_t *cr=obj->cr;
    // read buffer stuff
    static int cont=0;
    
    // cairo_select_font_face (cr, args->fontname, CAIRO_FONT_SLANT_NORMAL,
    // 			    CAIRO_FONT_WEIGHT_NORMAL);
    // cairo_set_font_size (cr, args->fontsize);
    pcobj_setfont(obj, args->fontname, args->fontsize);
    pcobj_font_face(obj, args->bfont_slant, args->bfont_weight);

    while ((!eof_u(in_f)) && (pline <= lineperpage)) {
	baseline = scoord->body_top+scoord->oneline_h*pline;
	if ((args->notebook)&&(pline<lineperpage)) {
	    // draw thin baseline
	    draw_rel_line(cr, mcoord->body_left, baseline, 
			  mcoord->bwidth, 0, 0.1, C_BASEL);
	}

        cairo_move_to(cr, mcoord->body_left, baseline);
        if (cont && args->fold_arrow) {
            // draw continue arrow
            double r = scoord->oneline_h - pcobj_font_ascent(obj)/2
                       + pcobj_font_descent(obj);
            if (args->numbering) {
                draw_cont_arrow
                    (cr, scoord->num_right-r/2, baseline-scoord->oneline_h, r,
                     ARROW_WIDTH, C_ARROW);
            } else {
                draw_cont_arrow
                    (cr, mcoord->body_left, baseline-scoord->oneline_h, r,
                     ARROW_WIDTH, C_ARROW);
            }
        } else {				
            if (args->numbering) {
                char nbuf[255];
                // draw line number
                snprintf(nbuf, 255, "%5d", fline);
                cairo_set_source_rgb(cr, C_NUMBER);                
                pcobj_move_to(obj, mcoord->body_left+scoord->body_inset, baseline);
                pcobj_print(obj, nbuf);
                fline++;
            }
        }
        limitw = mcoord->body_right - scoord->text_left - scoord->body_inset;
        cairo_set_source_rgb(cr, C_BLACK);
        // pcobj_move_to(obj, scoord->text_left, baseline);

        // folding & draw text
        draw_limited_text(obj, in_f, args->tab, limitw, &cont, scoord->text_left, baseline);
        //
			    
        if (cont && args->fold_arrow) {
            draw_return_arrow
                (cr, mcoord->body_right-scoord->body_inset, baseline-pcobj_font_ascent(obj)/2,
                 scoord->oneline_h-pcobj_font_ascent(obj)/2+pcobj_font_descent(obj),
                 ARROW_WIDTH, C_ARROW);
        }

	pline++;
    } // while ((!feof(in_f)) && (pline <= lineperpage))
    // finish body lines
		
    if (args->notebook) {
	int i;
	for (i=pline; i < lineperpage; i++){
	    baseline = scoord->body_top+scoord->oneline_h*i;
	    // thin baseline
	    draw_rel_line(cr, mcoord->body_left, baseline, 
			  mcoord->bwidth, 0, LW_THIN_BASELINE, C_BASEL);
	}
    }

    // settle font face default
    pcobj_font_face(obj, PANGO_STYLE_NORMAL, PANGO_WEIGHT_NORMAL);

} // end of draw_lines()


int draw_pages(cairo_t *cr, UFILE *in_f, args_t *args){
    // header
    char datebuf[255];
    struct tm *modt;
    // numbering
    int page=1;
    pcobj *obj=pcobj_new(cr);
    
    modt = localtime(args->mtime);
    strftime(datebuf, 255, args->date_format, modt);
    
    do {
	mcoord_t mc_store, *mcoord=&mc_store;
	scoord_t sc_store, *scoord=&sc_store;
	int lineperpage;

	// local coordinates
	double bottombase, bodyheight;
	    
	calc_page_coordinates(args, page, mcoord);

	scoord->body_inset = args->fontsize;
	// y-axis
	scoord->oneline_h = args->fontsize+args->betweenline;
	if (!args->noheader){
	    // has header
	    scoord->body_top = mcoord->head_top + args->header_height + scoord->oneline_h;
	    bodyheight = args->pheight - (scoord->body_top + mcoord->mbottom);
	} else {
	    // no header
	    scoord->body_top = mcoord->head_top;
	    bodyheight = args->pheight - (mcoord->head_top + mcoord->mbottom);
	}	    
	lineperpage = (int)(bodyheight/scoord->oneline_h);
	bottombase = scoord->body_top + scoord->oneline_h*lineperpage;
	    
	// x-axis
	// cairo_select_font_face (cr, args->fontname, CAIRO_FONT_SLANT_NORMAL,
        // 			CAIRO_FONT_WEIGHT_NORMAL);
	// cairo_set_font_size (cr, args->fontsize);
        pcobj_setfont(obj, args->fontname, args->fontsize);
        pcobj_font_face(obj, args->bfont_slant, args->bfont_weight);

	if (args->numbering) {
	    scoord->num_right = mcoord->body_left + scoord->body_inset
		+ pcobj_text_width(obj, "000000"); // vertical line
	    scoord->text_left = scoord->num_right + pcobj_text_width(obj, "0");
	} else {
	    scoord->text_left = mcoord->body_left + scoord->body_inset;
	}
	// draw punchmark
	if (args->punchmark){
	    switch (mcoord->markdir){
	    case d_none:
		break;
	    case d_up:
		draw_mark(cr, d_up, args->pwidth/2, mcoord->head_top/2);
		break;
	    case d_down:
		draw_mark(cr, d_down, args->pwidth/2, args->pheight - mcoord->mbottom/2);
		break;
	    case d_left:
		draw_mark(cr, d_left, mcoord->body_left/2, args->pheight/2);
		break;
	    case d_right:
		draw_mark(cr, d_right, (mcoord->body_right + args->pwidth)/2, args->pheight/2);
		break;
	    }
        }
        // draw border
        if (args->border){
            draw_rectangle(cr, mcoord->body_left, mcoord->head_top, mcoord->bwidth,
                           args->pheight - mcoord->mbottom - mcoord->head_top,
                           LW_BORDER, C_BORDER);
            if (!args->noheader){
                draw_rel_line(cr, mcoord->body_left, scoord->body_top,
                              mcoord->bwidth, 0, LW_BORDER, C_BORDER);
            }
        }
        // draw header
        if (!args->noheader){
            //
            draw_header(obj, args, page, mcoord, scoord, datebuf);
            //
            // header base line
            if (args->notebook){
                if (args->numbering) {
                    // virtical line
                    draw_rel_line(cr, scoord->num_right, scoord->body_top, 
                                  0, bottombase - scoord->body_top+LW_THICK_BASELINE,
                                  LW_VLINE, C_NUMVL);
                }
#if DEBUG_HOLDING
                draw_rel_line(cr, mcoord->body_left + scoord->body_inset,
                              scoord->body_top, 0, bottombase - scoord->body_top,
                              LW_VLINE, C_GREEN);
                draw_rel_line(cr, mcoord->body_right - scoord->body_inset,
                              scoord->body_top, 0, bottombase - scoord->body_top,
                              LW_VLINE, C_GREEN);
#endif
                // top line
                draw_rel_line(cr, mcoord->body_left, mcoord->head_top, mcoord->bwidth, 0,
                              LW_THICK_BASELINE, C_BASEL);
                draw_rel_line(cr, mcoord->body_left, scoord->body_top, mcoord->bwidth, 0,
                              LW_THICK_BASELINE, C_BASEL);
                cairo_set_source_rgb(cr, C_BLACK);
            }
        } else {
            // no header
            if (args->notebook) {
                // top line
                draw_rel_line(cr, mcoord->body_left, mcoord->head_top, mcoord->bwidth, 0,
                              1, C_BASEL);
                cairo_set_source_rgb(cr, C_BLACK);
                if (args->numbering) {
                    // vertical line
                    draw_rel_line(cr, scoord->num_right, mcoord->head_top,
                                  0, bottombase - mcoord->head_top+LW_THICK_BASELINE,
                                  LW_VLINE, C_NUMVL);
                }
            }
        } // if (!args->noheader)

        page++;
        
        // draw body
        draw_lines(obj, in_f, args, lineperpage, mcoord, scoord);
        //
	
        if (args->notebook){
            // footer line
            draw_rel_line(cr, mcoord->body_left, bottombase+1, mcoord->bwidth, 0, 1, C_BASEL);
        }

        if (!args->twocols || (page % 2 != 0)){
            if (!(eof_u(in_f))){
                cairo_show_page(cr); // new page
            }
        }
        // 
        // finish drawing one page.
    } while (! eof_u(in_f));
    pcobj_free(obj);
    
    page--;
    if (args->twocols) {
        return(ceil(page/2.0));
    } else {
        return(page);
    }
}   


// end of drawing.c
