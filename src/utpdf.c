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
#include "drawing.h"
#include "paper.h"
#include "usage.h"
#include "args.h"

int makepdf=1;
char *prog_name;

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


//
// 
int main(int argc, char** argv){
    int fileindex;
    // args_t *args;
    
    prog_name=path2cmd(argv[0]);
    makepdf = (strncmp(prog_name, MKPDFNAME, NAMELEN)==0);

    //
    // parse arguments
    //
    getargs(argc, argv);
    
    //
    // Draw each file
    //
    {
	// cairo stuff
	cairo_surface_t *surface=NULL;
	cairo_t *cr;
	int out_fd, output_notspecified=(args->outfile==NULL);
        int pages;
      
	for (fileindex = optind; fileindex < argc; fileindex++) {    
	    // file stuff
	    UFILE *in_f;
	    int in_fd;
	    struct stat stat_b;
  
	    // get input file name
	    args->in_fname = argv[fileindex];

	    // open input file
	    if (strncmp("-", args->in_fname, 3)==0){
		in_fd = STDIN_FILENO;
		args->in_fname="STDIN";
		args->current_t=1;
	    } else {
		in_fd = openfd(args->in_fname, O_RDONLY);
	    }
	    in_f = fdopen_u(in_fd, args->in_fname);

	    // create output file and surface 
	    if (surface == NULL) {
		// new file
		if (makepdf) {
		    if (output_notspecified) {
                        static char outf_store[255];
                        
 			snprintf(outf_store, 255, "%s.pdf", args->in_fname);
                        args->outfile = outf_store;
		    } 
                    if (strncmp(args->outfile, "-", 255)==0) {
			out_fd = STDOUT_FILENO;
		    } else {
			out_fd = openfd(args->outfile, O_CREAT|O_WRONLY|O_TRUNC);
		    }
		    surface = cairo_pdf_surface_create_for_stream
			((cairo_write_func_t )write_func, (void *)&out_fd,
			 args->pwidth, args->pheight);
		    cr = cairo_create(surface);
		} else {
                    // PostScript
		    if (output_notspecified) {
			// write to STDOUT
			args->outfile="-";
                        out_fd = STDOUT_FILENO;
		    } else if (strncmp(args->outfile, "-", 255)==0) {
			out_fd = STDOUT_FILENO;
		    } else {
			out_fd = openfd(args->outfile, O_CREAT|O_WRONLY|O_TRUNC);
		    }
		    surface = cairo_ps_surface_create_for_stream
			((cairo_write_func_t )write_func, (void *)&out_fd,
			 args->pwidth, args->pheight);
		    cr = cairo_create(surface);
                    if (args->duplex) {
                        if (args->longedge) {
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
  
            if (args->current_t){
                time(args->mtime);
            } else {
                if (fstat(in_fd, &stat_b)<0){
                    perror("Could not fstat: ");
                    exit(1);
                }
                *args->mtime = stat_b.st_mtime;
            }

            //
            pages=draw_pages(cr, in_f, args);
            //

            close_u(in_f);

            if (! args->one_output){
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

        if (args->one_output){
            // close output
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
            close(out_fd);
        }
    }
    exit(0);
}

// end of utpdf.c
