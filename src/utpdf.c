/*
  utpdf/utps
  margin-aware converter from utf-8 text to PDF/PostScript

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
#include "io.h"

int makepdf=1;
char *prog_name;

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
    
    setlocale(LC_ALL, "");
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
	// cairo_surface_t *surface=NULL;
        pcobj *obj=NULL;
	// cairo_t *cr;
	int out_fd, output_notspecified=(args->outfile==NULL);
      
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
	    if (obj == NULL) {
		// new file
		if (makepdf) {
		    if (output_notspecified) {
                        static char outf_store[S_LEN];
                        
 			snprintf(outf_store, S_LEN, "%s.pdf", args->in_fname);
                        args->outfile = outf_store;
		    } 
                    if (strncmp(args->outfile, "-", S_LEN)==0) {
			out_fd = STDOUT_FILENO;
		    } else {
			out_fd = openfd(args->outfile, O_CREAT|O_RDWR|O_TRUNC);
		    }
		    obj = pcobj_pdf_new
                        ((cairo_write_func_t )write_func, (void *)&out_fd, 
                         args->pwidth, args->pheight);
		} else {
                    // PostScript
		    if (output_notspecified) {
			// write to STDOUT
			args->outfile="-";
                        out_fd = STDOUT_FILENO;
		    } else if (strncmp(args->outfile, "-", S_LEN)==0) {
			out_fd = STDOUT_FILENO;
		    } else {
			out_fd = openfd(args->outfile, O_CREAT|O_WRONLY|O_TRUNC);
		    }
                    if (args->duplex) {
                        if (args->force_duplex){
                            obj = pcobj_ps_new
                                ((cairo_write_func_t )write_ps_duplex, (void *)&out_fd,
                                 args->phys_width, args->phys_height);
                        } else {
                            obj = pcobj_ps_new
                                ((cairo_write_func_t )write_func, (void *)&out_fd,
                                 args->phys_width, args->phys_height);
                        }
                        cairo_ps_surface_dsc_comment
                            (obj->surface, "%%Requirements: duplex");
                        cairo_ps_surface_dsc_begin_setup(obj->surface);
                        cairo_ps_surface_dsc_comment
                            (obj->surface,
                             "%%IncludeFeature: *Duplex DuplexNoTumble");
                        // set orientation
                        cairo_ps_surface_dsc_begin_page_setup (obj->surface);
                        if (args->portrait) {
                            cairo_ps_surface_dsc_comment
                                (obj->surface, "%%PageOrientation: Portrait");
                        } else {
                            cairo_ps_surface_dsc_comment
                                (obj->surface, "%%PageOrientation: Landscape");
                        }
                    } else {
                        // simplex printing
                        obj = pcobj_ps_new
                            ((cairo_write_func_t )write_func, (void *)&out_fd,
                             args->pwidth, args->pheight);
                        // set orientation
                        cairo_ps_surface_dsc_begin_page_setup (obj->surface);
                        if (args->portrait) {
                            cairo_ps_surface_dsc_comment
                                (obj->surface, "%%PageOrientation: Portrait");
                        } else {
                            cairo_ps_surface_dsc_comment
                                (obj->surface, "%%PageOrientation: Landscape");
                        }
                    } // if (args->duplex) else
		} // if (makepdf) else
                // cr = cairo_create(surface);
                // obj = pcobj_new(cr);
	    } // if (surface == NULL)
            
            cairo_set_source_rgb(obj->cr, C_BLACK);
  
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
            draw_file(obj, in_f, args, (fileindex == (argc-1)));
            //

            close_u(in_f);

            if (! args->one_output){
                // close output
                pcobj_free(obj);
                close(out_fd);
                obj=NULL;
            } else {
                // if (fileindex < (argc-1)){
                //    cairo_show_page(cr); // new page for next file.
                //}
            }
        } // for (fileindex = optind; fileindex < argc; fileindex++) {

        if (args->one_output){
            // close output
            pcobj_free(obj);
            close(out_fd);
        }
    }
    exit(0);
}

// end of utpdf.c
