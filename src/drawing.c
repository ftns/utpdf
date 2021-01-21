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
#include "coord.h"
#include "drawing.h"
#include "utpdf.h"
#include "args.h"

#define PI 3.14159265359


double font_ascent(cairo_t *cr){
    cairo_font_extents_t f_ext;

    cairo_font_extents(cr, &f_ext);
    return f_ext.ascent;
}

double font_descent(cairo_t *cr){
    cairo_font_extents_t f_ext;

    cairo_font_extents(cr, &f_ext);
    return f_ext.descent;
}

double text_width(cairo_t *cr, const char *str){
    cairo_text_extents_t t_ext;
    
    cairo_text_extents(cr, str, &t_ext);
    return t_ext.x_advance; // t_ext.width
}

void show_text_at_center(cairo_t *cr, const char *str){
    cairo_rel_move_to(cr, -text_width(cr, str)/2, 0);
    cairo_show_text(cr, str);
}

void show_text_at_right(cairo_t *cr, const char *str){
    cairo_rel_move_to(cr, -text_width(cr, str), 0);
    cairo_show_text(cr, str);
}

void show_text_at_left(cairo_t *cr, const char *str){
    cairo_show_text(cr, str);
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


void draw_header(cairo_t *cr, args_t *arg, int page, mcoord_t *mcoord,
                 scoord_t *scoord, char *datebuf){
    double hbaseline, hinset, gap;
    cairo_font_extents_t f_ext;
    char pagebuf[255];
    
    // hbaseline: the baseline of header font
    cairo_select_font_face (cr, arg->headerfont, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, arg->hfont_large);
    cairo_font_extents(cr, &f_ext);
    gap = (scoord->body_top - mcoord->head_top - (f_ext.ascent + f_ext.descent))/2;
    hbaseline = mcoord->head_top + gap + f_ext.ascent + f_ext.descent/2;

    // hinset: header inset
    cairo_set_font_size (cr, arg->hfont_medium);
    hinset = text_width(cr, "0");
		
    // draw left part: modified date
    cairo_select_font_face (cr, arg->headerfont, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, arg->hfont_medium);
    cairo_move_to(cr, mcoord->body_left+hinset, hbaseline);
    show_text_at_left(cr, datebuf);
  
    // center part: filename
    cairo_select_font_face (cr, arg->headerfont, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, arg->hfont_large);
    cairo_move_to(cr, mcoord->body_left+mcoord->bwidth/2, hbaseline);
    if (arg->headertext == NULL) {
	show_text_at_center(cr, arg->in_fname);
    } else {
	show_text_at_center(cr, arg->headertext);
    }
    // right part: page
    sprintf(pagebuf, "page: %d  ", page);
    cairo_select_font_face (cr, arg->headerfont, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, arg->hfont_medium);
    cairo_move_to(cr, mcoord->body_left + mcoord->bwidth - hinset, hbaseline);
    show_text_at_right(cr, pagebuf);
}


void draw_limited_text
    (cairo_t *cr, UFILE *in_f, int tab, const double limit, int *cont){
    int clen, olen;
    double em, tabw; // width of "M", tab
    double cur_left, orig_left, cur_base, limit_x;
    static char outbuf[BUFLEN], rbuf[8];
    static int over_sp=0;
    
    // outbuf=""
    outbuf[0] = '\0';
    olen = 0;

    cairo_get_current_point (cr, &cur_left, &cur_base);
    orig_left = cur_left;    
    limit_x = limit + cur_left;

    em=text_width(cr, "M");
    tabw=em*tab;
    cur_left+=em*over_sp;
    over_sp=0;
    cairo_move_to(cr, cur_left, cur_base);
    
    while((cur_left+text_width(cr, outbuf)<limit_x)
          && (get_one_uchar(in_f, rbuf)>0)
          && (rbuf[0] != 0x0D) && (rbuf[0] != 0x0A)){
	if (rbuf[0] == '\t') {
            // tab
	    double cur_right = cur_left+text_width(cr, outbuf);
	    double new_right = tabw*ceil((cur_right+em-orig_left)/tabw)+orig_left;
	    if (new_right < limit_x) {
		// tab jump
		cairo_show_text(cr, outbuf);
		cur_left = new_right;
		cairo_move_to(cr, cur_left, cur_base);
		outbuf[0] = '\0';
		olen = 0;
	    } else {
		// tab jump -> overflow
		over_sp = ceil((new_right-limit_x)/em); // over_sp is static.
		*cont=1;
		cairo_show_text(cr, outbuf);
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
    
    if (cur_left+text_width(cr, outbuf)>limit_x){
	// overflow
	*cont = 1;
	// remove 1char from outbuf, and print it
	outbuf[olen-clen] = '\0';
	cairo_show_text(cr, outbuf);
	// push back 1char.
	rewind_u(in_f, clen);
	return; 
    } else {
	// finish this line
	cairo_show_text(cr, outbuf);
	*cont = 0;
	return;
    }    
}


void draw_lines(cairo_t *cr, UFILE *in_f, args_t *arg, int lineperpage,
		mcoord_t *mcoord, scoord_t *scoord){
    int pline=1; 	// line number of this page
    static int fline=1; // line nunber of this file
    double limitw, baseline;
    // read buffer stuff
    static int cont=0;
    
    cairo_select_font_face (cr, arg->fontname, CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, arg->fontsize);
    
    while ((!eof_u(in_f)) && (pline <= lineperpage)) {
	baseline = scoord->body_top+scoord->oneline_h*pline;
	if ((arg->notebook)&&(pline<lineperpage)) {
	    // draw thin baseline
	    draw_rel_line(cr, mcoord->body_left, baseline, 
			  mcoord->bwidth, 0, 0.1, C_BASEL);
	}

        cairo_move_to(cr, mcoord->body_left, baseline);
        if (cont && arg->fold_arrow) {
            // draw continue arrow
            double r=scoord->oneline_h-font_ascent(cr)/2+font_descent(cr);
            if (arg->numbering) {
                draw_cont_arrow
                    (cr, scoord->num_right-r/2, baseline-scoord->oneline_h, r,
                     ARROW_WIDTH, C_ARROW);
            } else {
                draw_cont_arrow
                    (cr, mcoord->body_left, baseline-scoord->oneline_h, r,
                     ARROW_WIDTH, C_ARROW);
            }
        } else {				
            if (arg->numbering) {
                char nbuf[255];
                // draw line number
                cairo_move_to(cr, mcoord->body_left+scoord->body_inset, baseline);
                snprintf(nbuf, 255, "%5d", fline);
                cairo_set_source_rgb(cr, C_NUMBER);
                cairo_show_text(cr, nbuf);
                fline++;
            }
        }
        cairo_move_to(cr, scoord->text_left, baseline);
        limitw = mcoord->body_right - scoord->text_left - scoord->body_inset;
        cairo_set_source_rgb(cr, C_BLACK);

        // folding & draw text
        draw_limited_text(cr, in_f, arg->tab, limitw, &cont);
        //
			    
        if (cont && arg->fold_arrow) {
            draw_return_arrow
                (cr, mcoord->body_right-scoord->body_inset, baseline-font_ascent(cr)/2,
                 scoord->oneline_h-font_ascent(cr)/2+font_descent(cr),
                 ARROW_WIDTH, C_ARROW);
        }

	pline++;
    } // while ((!feof(in_f)) && (pline <= lineperpage))
    // finish body lines
		
    if (arg->notebook) {
	int i;
	for (i=pline; i < lineperpage; i++){
	    baseline = scoord->body_top+scoord->oneline_h*i;
	    // thin baseline
	    draw_rel_line(cr, mcoord->body_left, baseline, 
			  mcoord->bwidth, 0, LW_THIN_BASELINE, C_BASEL);
	}
    }
} // end of draw_lines()


int draw_pages(cairo_t *cr, UFILE *in_f, args_t *arg){
    // header
    char datebuf[255];
    struct tm *modt;
    // numbering
    int page=1;

    modt = localtime(arg->mtime);
    strftime(datebuf, 255, arg->date_format, modt);
    
    do {
	mcoord_t mc_store, *mcoord=&mc_store;
	scoord_t sc_store, *scoord=&sc_store;
	int lineperpage;

	// local coordinates
	double bottombase, bodyheight;
	    
	calc_page_coordinates(arg, page, mcoord);

	scoord->body_inset = arg->fontsize;
	// y-axis
	scoord->oneline_h = arg->fontsize+arg->betweenline;
	if (!arg->noheader){
	    // has header
	    scoord->body_top = mcoord->head_top + arg->header_height + scoord->oneline_h;
	    bodyheight = arg->pheight - (scoord->body_top + mcoord->mbottom);
	} else {
	    // no header
	    scoord->body_top = mcoord->head_top;
	    bodyheight = arg->pheight - (mcoord->head_top + mcoord->mbottom);
	}	    
	lineperpage = (int)(bodyheight/scoord->oneline_h);
	bottombase = scoord->body_top + scoord->oneline_h*lineperpage;
	    
	// x-axis
	cairo_select_font_face (cr, arg->fontname, CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, arg->fontsize);
	if (arg->numbering) {
	    scoord->num_right = mcoord->body_left + scoord->body_inset
		+ text_width(cr, "000000"); // vertical line
	    scoord->text_left = scoord->num_right + text_width(cr, "0");
	} else {
	    scoord->text_left = mcoord->body_left + scoord->body_inset;
	}
	// draw punchmark
	if (arg->punchmark){
	    switch (mcoord->markdir){
	    case d_none:
		break;
	    case d_up:
		draw_mark(cr, d_up, arg->pwidth/2, mcoord->head_top/2);
		break;
	    case d_down:
		draw_mark(cr, d_down, arg->pwidth/2, arg->pheight - mcoord->mbottom/2);
		break;
	    case d_left:
		draw_mark(cr, d_left, mcoord->body_left/2, arg->pheight/2);
		break;
	    case d_right:
		draw_mark(cr, d_right, (mcoord->body_right + arg->pwidth)/2, arg->pheight/2);
		break;
	    }
        }
        // draw border
        if (arg->border){
            draw_rectangle(cr, mcoord->body_left, mcoord->head_top, mcoord->bwidth,
                           arg->pheight - mcoord->mbottom - mcoord->head_top,
                           LW_BORDER, C_BORDER);
            if (!arg->noheader){
                draw_rel_line(cr, mcoord->body_left, scoord->body_top,
                              mcoord->bwidth, 0, LW_BORDER, C_BORDER);
            }
        }
        // draw header
        if (!arg->noheader){
            //
            draw_header(cr, arg, page, mcoord, scoord, datebuf);
            //
            // header base line
            if (arg->notebook){
                if (arg->numbering) {
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
            if (arg->notebook) {
                // top line
                draw_rel_line(cr, mcoord->body_left, mcoord->head_top, mcoord->bwidth, 0,
                              1, C_BASEL);
                cairo_set_source_rgb(cr, C_BLACK);
                if (arg->numbering) {
                    // vertical line
                    draw_rel_line(cr, scoord->num_right, mcoord->head_top,
                                  0, bottombase - mcoord->head_top+LW_THICK_BASELINE,
                                  LW_VLINE, C_NUMVL);
                }
            }
        } // if (!arg->noheader)

        page++;
        
        // draw body
        draw_lines(cr, in_f, arg, lineperpage, mcoord, scoord);
        //
	
        if (arg->notebook){
            // footer line
            draw_rel_line(cr, mcoord->body_left, bottombase+1, mcoord->bwidth, 0, 1, C_BASEL);
        }

        if (!arg->twosides || (page % 2 != 0)){
            if (!(eof_u(in_f))){
                cairo_show_page(cr); // new page
            }
        }
        // 
        // finish drawing one page.
    } while (! eof_u(in_f));
    page--;
    if (arg->twosides) {
        return(ceil(page/2.0));
    } else {
        return(page);
    }
}   


// end of drawing.c
