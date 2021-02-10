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

#include "utpdf.h"
#include "coord.h"

//
// calc main_coordinates

void twoside_oddpage_leftside(args_t *args, mcoord_t *mcoord){
  if (args->longedge) {
    if (args->portrait) {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth - (args->binding + args->outer + args->divide))/2;
      mcoord->body_left  = args->binding;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_left;
    } else {
      mcoord->head_top   = args->binding;
      mcoord->mbottom    = args->outer;
      mcoord->bwidth     = (args->pwidth-(args->ptop + args->pbottom + args->divide))/2;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_up;
    }
  } else {
    // short edge
    if (args->portrait) {
      mcoord->head_top   = args->binding;
      mcoord->mbottom    = args->outer;
      mcoord->bwidth     = (args->pwidth - (args->ptop + args->pbottom + args->divide))/2;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_up;
    } else {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth-(args->binding+args->outer+args->divide))/2;
      mcoord->body_left  = args->binding;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_left;
    }
  }
}

void twoside_oddpage_rightside(args_t *args, mcoord_t *mcoord){
  mcoord->markdir = d_none;
  if (args->longedge) {
    if (args->portrait) {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth - (args->binding + args->outer + args->divide))/2;
      mcoord->body_left  = args->binding + mcoord->bwidth + args->divide;
      mcoord->body_right = args->pwidth - args->outer;
    } else {
      mcoord->head_top   = args->binding;
      mcoord->mbottom    = args->outer;
      mcoord->bwidth     = (args->pwidth - (args->ptop + args->pbottom + args->divide))/2;
      mcoord->body_left  = args->ptop + mcoord->bwidth+args->divide;
      mcoord->body_right = args->pwidth - args->pbottom;
    }
  } else {
    // short edge
    if (args->portrait) {
      mcoord->head_top   = args->binding;
      mcoord->mbottom    = args->outer;
      mcoord->bwidth     = (args->pwidth - (args->ptop + args->pbottom + args->divide))/2;
      mcoord->body_left  = args->ptop + mcoord->bwidth + args->divide;
      mcoord->body_right = args->pwidth - args->pbottom;
    } else {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth - (args->binding + args->outer + args->divide))/2;
      mcoord->body_left  = args->binding + mcoord->bwidth + args->divide;
      mcoord->body_right = args->pwidth - args->outer;
    }
  }
}

void twoside_evenpage_leftside(args_t *args, mcoord_t *mcoord){
  if (args->longedge) {
    if (args->portrait) {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth - (args->binding + args->outer + args->divide))/2;
      mcoord->body_left  = args->outer;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_none; // d_right;
    } else {
      mcoord->head_top   = args->outer;
      mcoord->mbottom    = args->binding;
      mcoord->bwidth     = (args->pwidth - (args->ptop + args->pbottom + args->divide))/2;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_down;
    }
  } else {
    // short edge
    if (args->portrait) {
      mcoord->head_top   = args->outer;
      mcoord->mbottom    = args->binding;
      mcoord->bwidth     = (args->pwidth-(args->ptop+args->pbottom+args->divide))/2;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_down;
    } else {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth-(args->binding+args->outer+args->divide))/2;
      mcoord->body_left  = args->outer;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_none; // d_right;
    }
  }
}

void twoside_evenpage_rightside(args_t *args, mcoord_t *mcoord){
  if (args->longedge) {
    if (args->portrait) {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth-(args->binding+args->outer+args->divide))/2;
      mcoord->body_left  = args->outer+mcoord->bwidth+args->divide;
      mcoord->body_right = args->pwidth-args->binding;
      mcoord->markdir    = d_right;
    } else {
      mcoord->head_top   = args->outer;
      mcoord->mbottom    = args->binding;
      mcoord->bwidth     = (args->pwidth-(args->ptop+args->pbottom+args->divide))/2;
      mcoord->body_left  = args->ptop+mcoord->bwidth+args->divide;
      mcoord->body_right = args->pwidth-args->pbottom;
      mcoord->markdir    = d_none;
    }
  } else {
    // short edge
    if (args->portrait) {
      mcoord->head_top   = args->outer;
      mcoord->mbottom    = args->binding;
      mcoord->bwidth     = (args->pwidth-(args->ptop+args->pbottom+args->divide))/2;
      mcoord->body_left  = args->ptop+mcoord->bwidth+args->divide;
      mcoord->body_right = args->pwidth-args->pbottom;
      mcoord->markdir    = d_none;
    } else {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->bwidth     = (args->pwidth-(args->binding+args->outer+args->divide))/2;
      mcoord->body_left  = args->outer+mcoord->bwidth+args->divide;
      mcoord->body_right = args->pwidth-args->binding;
      mcoord->markdir    = d_right;
    }
  }
}

void oneside_oddpage(args_t *args, mcoord_t *mcoord){
  if (args->longedge) {
    if (args->portrait) {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->body_left  = args->binding;
      mcoord->body_right = args->pwidth-args->outer;
      mcoord->markdir =  d_left;
    } else {
      mcoord->head_top   = args->binding;
      mcoord->mbottom    = args->outer;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = args->pwidth-args->pbottom;
      mcoord->markdir    = d_up;
    }
  } else {
    // short edge
    if (args->portrait) {
      mcoord->head_top   = args->binding;
      mcoord->mbottom    = args->outer;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = args->pwidth-args->pbottom;
      mcoord->markdir    = d_up;
    } else {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->body_left  = args->binding;
      mcoord->body_right = args->pwidth - args->outer;
      mcoord->markdir    = d_left;
    }
  }
  mcoord->bwidth = mcoord->body_right - mcoord->body_left;
}

void oneside_evenpage(args_t *args, mcoord_t *mcoord){
  if (args->longedge) {
    if (args->portrait){
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->body_left  = args->outer;
      mcoord->body_right = args->pwidth-args->binding;
      mcoord->markdir    = d_right;
    } else {
      mcoord->head_top   = args->outer;
      mcoord->mbottom    = args->binding;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = args->pwidth-args->pbottom;
      mcoord->markdir    = d_down;
    }
  } else {
    if (args->portrait) {
      mcoord->head_top   = args->outer;
      mcoord->mbottom    = args->binding;
      mcoord->body_left  = args->ptop;
      mcoord->body_right = args->pwidth - args->pbottom;
      mcoord->markdir    = d_down;
    } else {
      mcoord->head_top   = args->ptop;
      mcoord->mbottom    = args->pbottom;
      mcoord->body_left  = args->outer;
      mcoord->body_right = args->pwidth - args->binding;
      mcoord->markdir    = d_right;
    }
  }
  mcoord->bwidth = mcoord->body_right - mcoord->body_left;
}


// calcurate coordinates depend on each page
void calc_page_coordinates(args_t *arg, int page, mcoord_t *mcoord){
    if (args->duplex){
        if (args->twocols){
            switch (page%4){
            case 1:
                // odd page, left side
                twoside_oddpage_leftside(arg, mcoord);
                break;
            case 2:
                // odd page, right side
                twoside_oddpage_rightside(arg, mcoord);
                break;
            case 3:
                // even page left side
                twoside_evenpage_leftside(arg, mcoord);
                break;
            case 0:
                // even page right side
                twoside_evenpage_rightside(arg, mcoord);
                break;
            } // switch (page%4)
        } else {
            // one side per page
            if (page%2){
                // odd page
                oneside_oddpage(arg, mcoord);
            } else {
                // even page
                oneside_evenpage(arg, mcoord);
            } // if (page %2)
            // mcoord->bwidth = mcoord->body_right - mcoord->body_left;
        } // if (args->twocols) else
    } else {
        if (args->twocols){
	    if (page%2){
	        // odd page only, left side
                twoside_oddpage_leftside(arg, mcoord);
  	    } else {
                // odd page only, right side
                twoside_oddpage_rightside(arg, mcoord);
 	    }
	} else {
	    // odd page only
            oneside_oddpage(arg, mcoord);
	}
    }   
}


void calc_page_subcoordinates(pcobj *obj, args_t *args, mcoord_t *mcoord, scoord_t *scoord){
    double bodyheight;
    
    scoord->body_inset = args->fontsize;
    // y-axis
    scoord->oneline_h = args->fontsize+args->betweenline;
    if (args->header){
        // has header
        scoord->body_top = mcoord->head_top + args->header_height + scoord->oneline_h;
        bodyheight = args->pheight - (scoord->body_top + mcoord->mbottom);
    } else {
        // no header
        scoord->body_top = mcoord->head_top;
        bodyheight = args->pheight - (mcoord->head_top + mcoord->mbottom);
    }	    
    scoord->lineperpage = (int)(bodyheight/scoord->oneline_h);
    scoord->bottombase = scoord->body_top + scoord->oneline_h*scoord->lineperpage;
	    
    // x-axis
    pcobj_setfont(obj, args->fontname, args->fontsize);
    pcobj_font_face(obj, args->bfont_slant, args->bfont_weight);

    if (args->numbering) {
        scoord->num_right = mcoord->body_left + scoord->body_inset
            + pcobj_text_width(obj, "000000"); // vertical line
        scoord->text_left = scoord->num_right + pcobj_text_width(obj, "0");
    } else {
        scoord->text_left = mcoord->body_left + scoord->body_inset;
    }
}


// end of coord.c
