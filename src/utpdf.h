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

#ifndef __UTPDF_H__
#define __UTPDF_H__

#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <locale.h>

#define MKPDFNAME "utpdf"
#define NAMELEN 255
#define VERSION "0.8"

// unit: point.
// 1inch = 72pt.
// 1mm   = 1/25.4inch = 2.835pt.

#define PTOP	36.00 // paper top: 0.5inch(12.7mm)
#define PBOTTOM	36.00 // paper bottom: 0.5inch(12.7mm)
#define BINDING	72.00 // binding: 1inch(25.4mm)
#define OUTER	36.00 // outer: 0.5inch(12.7mm)
#define DIVIDE	18.00 // divide: 0.25inch(6.35mm)

#define DEFAULT_FONT "IPAGothic"
#define FONTSIZE          9.8  	// default fontsize for oneside 
#define FONTSIZE_TWOSIDES 6.6	// default fontsize for twosides
#define BETWEEN_L         1.0	// space between lines

#define HEADER_FONT "sans-serif"
#define HFONT_LARGE  	    16.0    // default fontsize for header
#define HFONT_TWOSIDE_LARGE  9.0    // default fontsize for twosides header
#define HFONT_M_RATE	     0.75   // header medium font/large font

#define ARROW_WIDTH 1.0 // width of folding arrow

#define TAB     8
#define BUFLEN	1024

// punch mark size
#define MARK_H 4.0
#define MARK_W 2.0

// color
//               R    G    B
#define C_BASEL  0.6, 0.6, 1	// baseline
#define C_NUMBER 0.4, 0.4, 0.4  // line number
#define C_NUMVL  1,   0.6, 0.6  // vertical line along with left of line number
#define C_ARROW	 0.6, 0.6, 1    // folding arrow
#define C_BORDER 0.2, 0.2, 0.2  // border
#define C_WHITE  1,   1,   1	// white
#define C_RED    1,   0,   0	// red
#define C_GREEN  0,   1,   0	// green
#define C_BLUE   0,   0,   1	// blue
#define C_BLACK  0,   0,   0	// black

// line width
#define LW_THIN_BASELINE 0.1
#define LW_THICK_BASELINE 1
#define LW_VLINE 0.4
#define LW_BORDER 1

// header date format -- strftime()
// #define DATE_FORMAT "%m/%d/%y %H:%M"
#define DATE_FORMAT "%D %R"

enum direction { d_none, d_up, d_down, d_right, d_left  };

extern int makepdf;
extern char *prog_name;
extern char *path2cmd(char *p);
extern cairo_status_t write_func
   (void *closure, const unsigned char *data, unsigned int length);

#endif

// end of utpdf.h
