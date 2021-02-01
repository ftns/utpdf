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

#ifndef __COORD_H__
#define __COORD_H__

#include "utpdf.h"
#include "args.h"

typedef struct main_coordinates {
  double head_top, mbottom, body_left, body_right, bwidth;
  enum direction markdir;
} mcoord_t;

extern void calc_page_coordinates(args_t *args, int page, mcoord_t *mcoord);

#endif

// end of coord.h
