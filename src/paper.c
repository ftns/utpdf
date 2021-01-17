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

#include "paper.h"

struct psize paper_sizes[PAPERS_END]= {
    { "a3",     841.89, 1190.55 }, // a3: 297x420mm
    { "a4",     595.27,  841.89 }, // a4: 210x297mm
    { "a5",     419.52,  595.27 }, // a5: 148x210mm
    { "b3",    1031.81, 1459.84 }, // b3: 364x515mm
    { "b4",     728.50, 1031.81 }, // b4: 257x364mm
    { "b5",     515.90,  728.50 }, // b5: 182x257mm
    { "letter", 612.00,  792.00 }, // letter: 8.5x11inch
    { "legal",  612.00, 1008.00 }, // legal:  8.5x14inch
};


char *paper_default() {
    return paper_sizes[PNAME_DEFAULT].pname;
}
// end of paper.c
