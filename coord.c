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

void twoside_oddpage_leftside(struct arguments *arg, struct main_coordinates *mcoord){
  if (arg->longedge) {
    if (arg->portrait) {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth - (arg->binding + arg->outer + arg->divide))/2;
      mcoord->body_left  = arg->binding;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_left;
    } else {
      mcoord->head_top   = arg->binding;
      mcoord->mbottom    = arg->outer;
      mcoord->bwidth     = (arg->pwidth-(arg->ptop + arg->pbottom + arg->divide))/2;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_up;
    }
  } else {
    // short edge
    if (arg->portrait) {
      mcoord->head_top   = arg->binding;
      mcoord->mbottom    = arg->outer;
      mcoord->bwidth     = (arg->pwidth - (arg->ptop + arg->pbottom + arg->divide))/2;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_up;
    } else {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth-(arg->binding+arg->outer+arg->divide))/2;
      mcoord->body_left  = arg->binding;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_left;
    }
  }
}

void twoside_oddpage_rightside(struct arguments *arg, struct main_coordinates *mcoord){
  mcoord->markdir = d_none;
  if (arg->longedge) {
    if (arg->portrait) {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth - (arg->binding + arg->outer + arg->divide))/2;
      mcoord->body_left  = arg->binding + mcoord->bwidth + arg->divide;
      mcoord->body_right = arg->pwidth - arg->outer;
    } else {
      mcoord->head_top   = arg->binding;
      mcoord->mbottom    = arg->outer;
      mcoord->bwidth     = (arg->pwidth - (arg->ptop + arg->pbottom + arg->divide))/2;
      mcoord->body_left  = arg->ptop + mcoord->bwidth+arg->divide;
      mcoord->body_right = arg->pwidth - arg->pbottom;
    }
  } else {
    // short edge
    if (arg->portrait) {
      mcoord->head_top   = arg->binding;
      mcoord->mbottom    = arg->outer;
      mcoord->bwidth     = (arg->pwidth - (arg->ptop + arg->pbottom + arg->divide))/2;
      mcoord->body_left  = arg->ptop + mcoord->bwidth + arg->divide;
      mcoord->body_right = arg->pwidth - arg->pbottom;
    } else {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth - (arg->binding + arg->outer + arg->divide))/2;
      mcoord->body_left  = arg->binding + mcoord->bwidth + arg->divide;
      mcoord->body_right = arg->pwidth - arg->outer;
    }
  }
}

void twoside_evenpage_leftside(struct arguments *arg, struct main_coordinates *mcoord){
  if (arg->longedge) {
    if (arg->portrait) {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth - (arg->binding + arg->outer + arg->divide))/2;
      mcoord->body_left  = arg->outer;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_none; // d_right;
    } else {
      mcoord->head_top   = arg->outer;
      mcoord->mbottom    = arg->binding;
      mcoord->bwidth     = (arg->pwidth - (arg->ptop + arg->pbottom + arg->divide))/2;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = mcoord->body_left + mcoord->bwidth;
      mcoord->markdir    = d_down;
    }
  } else {
    // short edge
    if (arg->portrait) {
      mcoord->head_top   = arg->outer;
      mcoord->mbottom    = arg->binding;
      mcoord->bwidth     = (arg->pwidth-(arg->ptop+arg->pbottom+arg->divide))/2;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_down;
    } else {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth-(arg->binding+arg->outer+arg->divide))/2;
      mcoord->body_left  = arg->outer;
      mcoord->body_right = mcoord->body_left+mcoord->bwidth;
      mcoord->markdir    = d_none; // d_right;
    }
  }
}

void twoside_evenpage_rightside(struct arguments *arg, struct main_coordinates *mcoord){
  if (arg->longedge) {
    if (arg->portrait) {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth-(arg->binding+arg->outer+arg->divide))/2;
      mcoord->body_left  = arg->outer+mcoord->bwidth+arg->divide;
      mcoord->body_right = arg->pwidth-arg->binding;
      mcoord->markdir    = d_right;
    } else {
      mcoord->head_top   = arg->outer;
      mcoord->mbottom    = arg->binding;
      mcoord->bwidth     = (arg->pwidth-(arg->ptop+arg->pbottom+arg->divide))/2;
      mcoord->body_left  = arg->ptop+mcoord->bwidth+arg->divide;
      mcoord->body_right = arg->pwidth-arg->pbottom;
      mcoord->markdir    = d_none;
    }
  } else {
    // short edge
    if (arg->portrait) {
      mcoord->head_top   = arg->outer;
      mcoord->mbottom    = arg->binding;
      mcoord->bwidth     = (arg->pwidth-(arg->ptop+arg->pbottom+arg->divide))/2;
      mcoord->body_left  = arg->ptop+mcoord->bwidth+arg->divide;
      mcoord->body_right = arg->pwidth-arg->pbottom;
      mcoord->markdir    = d_none;
    } else {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->bwidth     = (arg->pwidth-(arg->binding+arg->outer+arg->divide))/2;
      mcoord->body_left  = arg->outer+mcoord->bwidth+arg->divide;
      mcoord->body_right = arg->pwidth-arg->binding;
      mcoord->markdir    = d_right;
    }
  }
}

void oneside_oddpage( struct arguments *arg, struct main_coordinates *mcoord){
  if (arg->longedge) {
    if (arg->portrait) {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->body_left  = arg->binding;
      mcoord->body_right = arg->pwidth-arg->outer;
      mcoord->markdir =  d_left;
    } else {
      mcoord->head_top   = arg->binding;
      mcoord->mbottom    = arg->outer;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = arg->pwidth-arg->pbottom;
      mcoord->markdir    = d_up;
    }
  } else {
    // short edge
    if (arg->portrait) {
      mcoord->head_top   = arg->binding;
      mcoord->mbottom    = arg->outer;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = arg->pwidth-arg->pbottom;
      mcoord->markdir    = d_up;
    } else {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->body_left  = arg->binding;
      mcoord->body_right = arg->pwidth - arg->outer;
      mcoord->markdir    = d_left;
    }
  }
  mcoord->bwidth = mcoord->body_right - mcoord->body_left;
}

void oneside_evenpage(struct arguments *arg, struct main_coordinates *mcoord){
  if (arg->longedge) {
    if (arg->portrait){
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->body_left  = arg->outer;
      mcoord->body_right = arg->pwidth-arg->binding;
      mcoord->markdir    = d_right;
    } else {
      mcoord->head_top   = arg->outer;
      mcoord->mbottom    = arg->binding;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = arg->pwidth-arg->pbottom;
      mcoord->markdir    = d_down;
    }
  } else {
    if (arg->portrait) {
      mcoord->head_top   = arg->outer;
      mcoord->mbottom    = arg->binding;
      mcoord->body_left  = arg->ptop;
      mcoord->body_right = arg->pwidth - arg->pbottom;
      mcoord->markdir    = d_down;
    } else {
      mcoord->head_top   = arg->ptop;
      mcoord->mbottom    = arg->pbottom;
      mcoord->body_left  = arg->outer;
      mcoord->body_right = arg->pwidth - arg->binding;
      mcoord->markdir    = d_right;
    }
  }
  mcoord->bwidth = mcoord->body_right - mcoord->body_left;
}


// calcurate coordinates depend on each page
void calc_page_coordinates(struct arguments *arg, int page,
			   struct main_coordinates *mcoord){
    if (arg->duplex){
        if (arg->twosides){
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
        } // if (arg->twosides) else
    } else {
        if (arg->twosides){
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

// end of coord.c
