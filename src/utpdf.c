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

#define DEBUG_HOLDING 0

#include "utpdf.h"
#include "drawing.h"
#include "paper.h"
#include "usage.h"

int makepdf=1;


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

char *path2cmd(char *p){
    char *cur=p;
    while (*cur != '\0') {
        if (*cur == '/') p=cur+1;
        cur++;
    }
    return p;
}

int chk_sw(char *str, char *positive, char *negative) {
    if (str==NULL) { return 1; }
    else if (strncmp(str, positive, 8)==0) { return 1; }
    else if (strncmp(str, negative, 8)==0) { return 0; }
    else {
        USAGE("argument must be \"%s\" or \"%s\"\n", positive, negative);
        return -1;
    }
}

int chk_onoff(char *str){
    return chk_sw(str, "on", "off");
}

//
// 
int main(int argc, char** argv){
    int fileindex;

    char *outfile=NULL, *binding_dir=NULL;
    char *paper=NULL;
    double headersize=0.0;
    int current_t=0, inch=0, one_output=0;
	
    time_t mtime_store;
    
    struct arguments arg_store={
        // option flags
	.twosides=-1, .numbering=0, .noheader=0, .punchmark=0, .duplex=1,
	.portrait=1, .longedge=0, .tab=TAB, .notebook=0, .fold_arrow=1,
        .border=0,
        // ootion strings
	.fontname=NULL, .headerfont=NULL, /* in_fname, */ .date_format=DATE_FORMAT,
        .headertext=NULL,
        // option length
	.fontsize=0.0, .hfont_large=0, /* hfont_medium, header_height, */
        // paper size and margins
        /* pwidth, pheight, */ .binding=0.0, .outer=0.0, 
	.ptop=0.0, .pbottom=0.0, .divide=0.0, .betweenline=BETWEEN_L,
        // file modified time
        .mtime=&mtime_store
    };
    struct arguments *arg=&arg_store;
    
    // for getopt_long()
    int opt;

    enum i_option
    { i_help, i_inch, i_mm, i_binding, i_outer, i_top, i_bottom, i_divide, 
      i_hfont, i_hsize, i_notebk, i_datefmt, i_headtxt, i_version, i_header,
      i_fold_a, i_time_s, i_border, i_punch, i_number, i_binddir, i_side,
      i_unit, i_END };

    struct option long_options[] = {
	/* i_help    */ { "help",        no_argument,          0, 'h'},
	/* i_inch    */ { "inch",        no_argument,      &inch,  1 },
	/* i_mm      */ { "mm",          no_argument,      &inch,  0 },
	/* i_binding */ { "binding",     required_argument,    0,  0 },
	/* i_outer   */ { "outer",       required_argument,    0,  0 },
	/* i_top     */ { "top",         required_argument,    0,  0 },
	/* i_bottom  */ { "bottom",      required_argument,    0,  0 },
	/* i_divide  */ { "divide",      required_argument,    0,  0 },
	/* i_hfont   */ { "header-font", required_argument,    0,  0 },
	/* i_hsize   */ { "header-size", required_argument,    0,  0 },        
	/* i_notebk  */ { "notebook",    optional_argument,    0,  0 },
	/* i_datefmt */ { "date-format", required_argument,    0,  0 },
	/* i_headtxt */ { "header-text", required_argument,    0,  0 },
	/* i_version */ { "version",     no_argument,          0, 'V'},
        /* i_header  */ { "header",      optional_argument,    0,  0 },
        /* i_fold_a  */ { "fold-arrow",  optional_argument,    0,  0 },
        /* i_time_s  */ { "timestamp",   required_argument,    0,  0 },
        /* i_border  */ { "border",      optional_argument,    0,  0 },
        /* i_punch   */ { "punch",       optional_argument,    0,  0 },
        /* i_number  */ { "number",      optional_argument,    0,  0 },
        /* i_binddir */ { "binddir",     required_argument,    0,  0 },
        /* i_side    */ { "side",        required_argument,    0,  0 },
        /* i_unit    */ { "unit",        required_argument,    0,  0 },
	/* i_END     */ { 0, 0, 0, 0 },
    };
    int lindex;

    //
    // parse arguments
    //
    
    prog_name=path2cmd(argv[0]);

    makepdf = (strncmp(prog_name, MKPDFNAME, NAMELEN)==0);
    
    while ((opt = getopt_long
            (argc, argv, "12bB:dF:hlmno:pP:sS:t:V", long_options, &lindex)) != -1){
	if (opt == 0) {
	    // long option
	    switch (lindex) {
            case i_binddir:
                binding_dir=optarg;
                break;
	    case i_binding:
		if (sscanf(optarg, "%lf", &arg->binding)<1) {
                    USAGE("--binding argument:%s was wrong. \nExample: --binding=25.4\n", optarg);
                }
                break;
	    case i_outer:
		if (sscanf(optarg, "%lf", &arg->outer)<1) {
                    USAGE("--outer argument:%s was wrong. \nExample: --outer=12.7\n", optarg);
                }
                break;
	    case i_top:
		if (sscanf(optarg, "%lf", &arg->ptop)<1) {
                    USAGE("--top argument:%s was wrong. \nExample: --top=12.7\n", optarg);
                }
                break;
	    case i_bottom:
		if (sscanf(optarg, "%lf", &arg->pbottom)<1) {
                    USAGE("--bottom argument:%s was wrong. \nExample: --bottom=12.7\n", optarg);
                }
                break;
	    case i_divide:
		if (sscanf(optarg, "%lf", &arg->divide)<1) {
                    USAGE("--divide argument:%s was wrong. \nExample: --divide=12.7\n", optarg);
                }
                break;
	    case i_hfont:
		arg->headerfont = optarg; break;
	    case i_hsize:
		if (sscanf(optarg, "%lf", &headersize)<1)  {
                    USAGE("--header-size argument:%s was wrong. \nExample: --header-size=12.0\n", optarg);
                }
                break;
            case i_notebk:
                arg->notebook = chk_onoff(optarg); break;
	    case i_datefmt:
		arg->date_format = optarg; break;
	    case i_headtxt:
		arg->headertext = optarg; break;
            case i_header:
                arg->noheader = !chk_onoff(optarg); break;
            case i_fold_a:
                arg->fold_arrow = chk_onoff(optarg); break;
            case i_time_s:
                current_t = chk_sw(optarg, "cur", "mod"); break;
            case i_border:
                arg->border = chk_onoff(optarg); break;
            case i_punch:
                arg->punchmark = chk_onoff(optarg); break;
            case i_number:
                arg->numbering = chk_onoff(optarg); break;
            case i_side:
                arg->twosides = chk_sw(optarg, "2", "1"); break;
            case i_unit:
                inch = chk_sw(optarg, "inch", "mm"); break;
	    } // switch (lindex)
	} else {
	    // short option
	    switch (opt) {
	    case '1':
		arg->twosides=0; break;
	    case '2':
		arg->twosides=1; break;
	    case 'b':
                arg->border = 1; break;
	    case 'B':
		if (sscanf(optarg, "%lf", &arg->betweenline)<1) {
                    usage("-B:(space between lines) argument was wrong\nExample: -B 1.5\n");
                }
                break;
	    case 'd':
		arg->duplex=1; break;
	    case 'F':
		arg->fontname = optarg; break;	
	    case 'h':
		help(); break;
	    case 'H':
		arg->noheader=1; break;
	    case 'l':
		arg->portrait=0; break;	
	    case 'm':
		arg->punchmark=1; break;
	    case 'n':
		arg->numbering=1; break;
	    case 'o':
		outfile = optarg; one_output=1; break;
	    case 'p':
		arg->portrait=1; break;
	    case 'P':
		paper = optarg; break;
	    case 's':
		arg->duplex=0; break;
	    case 'S':
		if (sscanf(optarg, "%lf", &arg->fontsize)<1) {
                    USAGE("-S:(font size in pt.) argument:\"%s\" was wrong.\nExample: -S 12.0\n", optarg);
                }
                break;
	    case 't':
		if (sscanf(optarg, "%d", &(arg->tab))<1) {
                    USAGE("-t:(tab width) argument:\"%s\" was wrong.\nExample: -t 8\n", optarg);
                }
                break;
            case 'V':
                version(); break;
	    default:
		// error
		USAGE("Unknown argument:-%c\n", opt);
	    } // switch(opt)
	}
    }

    // input files are exist?
    if (optind >= argc) {
	usage("No file specified\n");
    }
    // twoside default setting
    if (arg->twosides == -1) {
	arg->twosides = (!arg->portrait);
    }
    // font size
    if (arg->fontsize == 0.0){
	if (!arg->twosides) {
	    arg->fontsize = FONTSIZE;
	} else {
	    arg->fontsize = FONTSIZE_TWOSIDES;
	}
    }

    if (headersize == 0.0){
	if (! arg->twosides) {
	    arg->hfont_large = HFONT_LARGE;
	} else {
	    arg->hfont_large = HFONT_TWOSIDE_LARGE;
	}
    } else {
	arg->hfont_large = headersize;
    }
    arg->hfont_medium = arg->hfont_large*HFONT_M_RATE;
    arg->header_height = arg->hfont_large;

    // font name
    if (arg->fontname == NULL) {
	arg->fontname=DEFAULT_FONT;
    }

    if (arg->headerfont == NULL) {
	arg->headerfont = HEADER_FONT;
    }
    
    // paper
    {
	int p;
	if (paper != NULL){
	    for (p=0; p<PAPERS_END; p++){
		if (strncmp(paper, paper_sizes[p].pname, PNAME_SIZE)==0){
		    if (arg->portrait){
			arg->pwidth  = paper_sizes[p].w;
			arg->pheight = paper_sizes[p].h;
		    } else {
			arg->pwidth  = paper_sizes[p].h;
			arg->pheight = paper_sizes[p].w;
		    }
		    break; 
		}
	    }
	    if (p>=PAPERS_END) USAGE("Unknown paper:%s\n", paper);
	} else {
	    // default
	    if (arg->portrait){
		arg->pwidth  = paper_sizes[PNAME_DEFAULT].w;
		arg->pheight = paper_sizes[PNAME_DEFAULT].h;
	    } else {
		arg->pwidth  = paper_sizes[PNAME_DEFAULT].h;
		arg->pheight = paper_sizes[PNAME_DEFAULT].w;
	    }
	}
    } // end of paper

    // margins
    if (inch) {
	// inch -> point
	arg->binding *= 72; arg->outer   *= 72;
	arg->ptop    *= 72; arg->pbottom *= 72;
	arg->divide  *= 72;
    } else {
	// mm -> point
	arg->binding *= 2.8346; arg->outer   *= 2.8346;
	arg->ptop    *= 2.8346; arg->pbottom *= 2.8346;
	arg->divide  *= 2.8346;
    }
    if (arg->binding == 0) { arg->binding = BINDING; };
    if (arg->outer   == 0) { arg->outer   = OUTER;   };
    if (arg->ptop    == 0) { arg->ptop    = PTOP;    };
    if (arg->pbottom == 0) { arg->pbottom = PBOTTOM; };
    if (arg->divide  == 0) { arg->divide  = PBOTTOM; };
    
    if (binding_dir == NULL){ // default
	arg->longedge = arg->portrait; // shortedge = landscape(!arg->portrait)
    } else {
	switch (*binding_dir){
	case 'l': // long edge
	    arg->longedge = 1;
	    break;
	case 's': // short edge
	    arg->longedge = 0;
	    break;
	case 'n': // no binding edge
	    arg->longedge = 1;
	    arg->binding = arg->outer;
	    break;
	default:
	    USAGE("--binding must be \'l/s/n\', but \'%c\'\n", *binding_dir);
	}
    }
    // end of margins

    
    //
    // Draw each file
    //
    {
	// cairo stuff
	cairo_surface_t *surface=NULL;
	cairo_t *cr;
	int out_fd, output_notspecified=(outfile==NULL);
        int pages;
      
	for (fileindex = optind; fileindex < argc; fileindex++) {    
	    // file stuff
	    struct UFILE *in_f;
	    int in_fd;
	    struct stat stat_b;
  
	    // get input file name
	    arg->in_fname = argv[fileindex];

	    // open input file
	    if (strncmp("-", arg->in_fname, 3)==0){
		in_fd = STDIN_FILENO;
		arg->in_fname="STDIN";
		current_t=1;
	    } else {
		in_fd = openfd(arg->in_fname, O_RDONLY);
	    }
	    in_f = fdopen_u(in_fd, arg->in_fname);

	    // create output file and surface 
	    if (surface == NULL) {
		// new file
		if (makepdf) {
		    if (output_notspecified) {
                        static char outfile_store[255];
                        
 			snprintf(outfile_store, 255, "%s.pdf", arg->in_fname);
                        outfile = outfile_store;
		    } 
                    if (strncmp(outfile, "-", 255)==0) {
			out_fd = STDOUT_FILENO;
		    } else {
			out_fd = openfd(outfile, O_CREAT|O_WRONLY|O_TRUNC);
		    }
		    surface = cairo_pdf_surface_create_for_stream
			((cairo_write_func_t )write_func, (void *)&out_fd,
			 arg->pwidth, arg->pheight);
		    cr = cairo_create(surface);
		} else {
                    // PostScript
		    if (output_notspecified) {
			// write to STDOUT
			outfile="-";
                        out_fd = STDOUT_FILENO;
		    } else if (strncmp(outfile, "-", 255)==0) {
			out_fd = STDOUT_FILENO;
		    } else {
			out_fd = openfd(outfile, O_CREAT|O_WRONLY|O_TRUNC);
		    }
		    surface = cairo_ps_surface_create_for_stream
			((cairo_write_func_t )write_func, (void *)&out_fd,
			 arg->pwidth, arg->pheight);
		    cr = cairo_create(surface);
                    if (arg->duplex) {
                        if (arg->longedge) {
                            cairo_ps_surface_dsc_comment
                                (surface, "%%Requirements: duplex");
                            cairo_ps_surface_dsc_begin_setup(surface);
                            cairo_ps_surface_dsc_comment
                                (surface, "%%IncludeFeature: *Duplex DuplexNoTumble");
                        } else {
                            cairo_ps_surface_dsc_comment
                                (surface, "%%Requirements: duplex");
                            cairo_ps_surface_dsc_begin_setup(surface);
                            cairo_ps_surface_dsc_comment
                                (surface, "%%IncludeFeature: *Duplex DuplexTumble");
                        }
                    }
		} // if (makepdf) else 
	    } // if (surface == NULL)
            
            cairo_set_source_rgb(cr, C_BLACK);
  
            if (current_t){
                time(arg->mtime);
            } else {
                if (fstat(in_fd, &stat_b)<0){
                    perror("Could not fstat: ");
                    exit(1);
                }
                *arg->mtime = stat_b.st_mtime;
            }

            //
            pages=draw_pages(cr, in_f, arg);
            //

            close_u(in_f);

            if (! one_output){
                // close output
                cairo_destroy(cr);
                cairo_surface_destroy(surface);
                close(out_fd);
                surface=NULL;
            } else {
                cairo_show_page(cr); // new page
                if (pages%2) {
                    cairo_show_page(cr); // add odd page
                }
            }
        } // for (fileindex = optind; fileindex < argc; fileindex++) {

        if (one_output){
            // close output
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
            close(out_fd);
        }
    }
    exit(0);
}

// end of utpdf.c
