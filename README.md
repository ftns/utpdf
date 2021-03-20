# utpdf/utps
a margin-aware converter from utf-8 text to PDF/PostScript

## Description

Convert utf-8 text files or standard input to PDF/Postscript.
By default, the PDF output is stored in files with ".pdf" suffix,
PostScript output is sent to standard output.

utpdf/utps set suitable margins with simplex/duplex, portrait/landscape,
long edge binded/short edge binded printing. So printed papers
are filed correctly in binder.

utpdf/utps shows timestamp on the header. By default, it is
a modification time of files. If you specified by "--timestamp=cur"
option, or input is standard input, the timestamp is current time.

## Requirements
* Cairo
    - https://cairographics.org/
* Pango
    - https://www.pango.org/
* pkg-config
    - https://freedesktop.org/wiki/Software/pkg-config/

## Change Log
* 3/20/2021
  * temporal release version(v0.8.3)
* 1/17/2021
  * initial version(v0.8)
