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
#include "args.h"
#include "paper.h"
#include "usage.h"

#define USAGE(args...) { char buf[255]; snprintf(buf, 255, args); usage(buf);}
typedef void (*usage_func_t)(char *);

time_t mtime_store;
args_t args_store = {
    // option flags
    .twosides=-1, .numbering=0, .noheader=0, .punchmark=0, .duplex=1,
    .portrait=1, .longedge=0, .tab=TAB, .notebook=0, .fold_arrow=1,
    .border=0, .current_t=0, .one_output=0, .inch=0,
    .hfont_slant=CAIRO_FONT_SLANT_NORMAL, .hfont_weight=CAIRO_FONT_WEIGHT_BOLD,
    // option strings
    .fontname=NULL, .headerfont=NULL, /* in_fname, */ .date_format=DATE_FORMAT,
    .headertext=NULL, .outfile=NULL, .binding_dir=NULL, .paper=NULL,
    // option length
    .fontsize=0, .hfont_large=0, /* hfont_medium, header_height, */
    .headersize=0,
    // paper size and margins
    /* pwidth, pheight, */ .binding=0, .outer=0, .ptop=0, .pbottom=0,
    .divide=0, .betweenline=BETWEEN_L,
    // file modified time
    .mtime=&mtime_store,
};
args_t *args = &args_store;

typedef enum i_option
{ i_help, i_version, i_inch, i_mm, i_binding, i_outer, i_top, i_bottom, 
  i_divide, i_hfont, i_hsize, i_notebk, i_datefmt, i_headtxt, i_header,
  i_fold_a, i_time_s, i_border, i_punch, i_number, i_binddir, i_side,
  i_unit, i_orient, i_hslant, i_hweigbt, i_END } i_option_t;

struct option long_options[]={
        /*               { char *name; int has_arg; int *flag; int val; }; */
        /* 00 i_help    */ { "help",          no_argument,          0, 'h'},
        /* 01 i_version */ { "version",       no_argument,          0, 'V'},
        /* 02 i_inch    */ { "inch",     no_argument, &args_store.inch, 1 },
        /* 03 i_mm      */ { "mm",       no_argument, &args_store.inch, 0 },
        /*    below options appeares in config file. */
        /* 04 i_binding */ { "binding",       required_argument,    0,  0 },
        /* 05 i_outer   */ { "outer",         required_argument,    0,  0 },
        /* 06 i_top     */ { "top",           required_argument,    0,  0 },
        /* 07 i_bottom  */ { "bottom",        required_argument,    0,  0 },
        /* 08 i_divide  */ { "divide",        required_argument,    0,  0 },
        /* 09 i_hfont   */ { "header-font",   required_argument,    0,  0 },
        /* 10 i_hsize   */ { "header-size",   required_argument,    0,  0 },        
        /* 11 i_notebk  */ { "notebook",      optional_argument,    0,  0 },
        /* 12 i_datefmt */ { "date-format",   optional_argument,    0,  0 },
        /* 13 i_headtxt */ { "header-text",   optional_argument,    0,  0 },
        /* 14 i_header  */ { "header",        optional_argument,    0,  0 },
        /* 15 i_fold_a  */ { "fold-arrow",    optional_argument,    0,  0 },
        /* 16 i_time_s  */ { "timestamp",     required_argument,    0,  0 },
        /* 17 i_border  */ { "border",        optional_argument,    0,  0 },
        /* 18 i_punch   */ { "punch",         optional_argument,    0,  0 },
        /* 19 i_number  */ { "number",        optional_argument,    0,  0 },
        /* 20 i_binddir */ { "binddir",       required_argument,    0,  0 },
        /* 21 i_side    */ { "side",          required_argument,    0,  0 },
        /* 22 i_unit    */ { "unit",          required_argument,    0,  0 },
        /* 23 i_orient  */ { "orientation",   required_argument,    0,  0 },
        /* 24 i_hslant  */ { "header-slant",  required_argument,    0,  0 },
        /* 25 i_hweigbt */ { "header-weight", required_argument,    0,  0 },
        /* 26 i_END     */ { 0, 0, 0, 0 }
};

#define LONGOP_NAMELEN 16
#define PARSE_LEN 256

char *conf_path="";
int conf_line;
//
// forward declaration
void conf_usage(char *message);
int  chk_shortop(struct option *loption, char *value);
void read_config(char *path);
void parser(int short_index, int long_index, char *argstr, usage_func_t usage);
void chk_slant(int *value, char *str, usage_func_t usage);
void chk_weight(int *value, char *str, usage_func_t usage);
int  chk_sw(char *str, char *positive, char *negative, usage_func_t usage);
int  chk_onoff(char *str, usage_func_t usage);
char *getconfpath();
int  parse_conf(char *str, char *key, char *value);
//
//

void conf_usage(char *message){
    if (message != NULL){
        fprintf(stderr, "%s line %d: %s\n", conf_path, conf_line, message);
    }
}

int chk_shortop(struct option *loption, char *value){
    if ((loption->flag == NULL) && (loption->val != 0)){
        parser(loption->val, 0, value, conf_usage);
        return 1;
    }
    if (loption->flag!=NULL){
        *loption->flag=loption->val;
        return 1;
    }
    return 0;
}

    
void read_config(char *path){
    usage_func_t usage = conf_usage;
    FILE *f=fopen(path, "r");
    
    if (f==NULL) {
        // no config file.
        return;
    }
    conf_path=path;
    conf_line=0;
    while (!feof(f)){
        char linebuf[PARSE_LEN], key[PARSE_LEN], *value=(char *)malloc(PARSE_LEN);
        int index, count;

        conf_line++;
        fgets(linebuf, PARSE_LEN, f);
        count=parse_conf(linebuf, key, value);
        // fprintf(stderr, "%d, \"%s\", \"%s\"\n", count, key, value); // DEBUG
        
        if ((count < 1)||(key[0]=='#')){
            continue; // skip this line.
        }
        // compare key and long_options[index].name from i_binding to i_END-1.
        index = i_binding;
        while (index < i_END){
            if (strncmp(key, long_options[index].name, LONGOP_NAMELEN)==0) break;
            index++;
        }
        if (index < i_END){
            // keyword hit!
            switch (long_options[index].has_arg){
            case no_argument:
                if (chk_shortop(&long_options[index], NULL)) break;
                parser(0, index, NULL, conf_usage);
                break;
            case required_argument:
                if (chk_shortop(&long_options[index], NULL)) break;
                if (count==2){
                    parser(0, index, value, conf_usage);
                } else {
                    USAGE("%s require an argument\n", key);
                }
                break;
            case optional_argument:
                if (chk_shortop(&long_options[index], NULL)) break;
                if (count==1) {
                    parser(0, index, NULL, conf_usage); 
                } else {
                    parser(0, index, value, conf_usage);
                }
                break;
            } // switch()
        } else {
            USAGE("%s: No such option.\n", key);
        } // if (index < i_end) else
    }
    fclose(f);
    conf_path="";
}

void parser(int short_index, int long_index, char *argstr, usage_func_t usage){
    if (short_index == 0) {
        // long option
        switch (long_index) {
        case i_binddir:
            args->binding_dir=argstr;
            break;
        case i_binding:
            if (sscanf(argstr, "%lf", &args->binding)<1) {
                USAGE("--binding argument:%s was wrong. \nExample: --binding=25.4\n", argstr);
            }
            break;
        case i_outer:
            if (sscanf(argstr, "%lf", &args->outer)<1) {
                USAGE("--outer argument:%s was wrong. \nExample: --outer=12.7\n", argstr);
            }
            break;
        case i_top:
            if (sscanf(argstr, "%lf", &args->ptop)<1) {
                USAGE("--top argument:%s was wrong. \nExample: --top=12.7\n", argstr);
            }
            break;
        case i_bottom:
            if (sscanf(argstr, "%lf", &args->pbottom)<1) {
                USAGE("--bottom argument:%s was wrong. \nExample: --bottom=12.7\n", argstr);
            }
            break;
        case i_divide:
            if (sscanf(argstr, "%lf", &args->divide)<1) {
                USAGE("--divide argument:%s was wrong. \nExample: --divide=12.7\n", argstr);
            }
            break;
        case i_hfont:
            args->headerfont = argstr; break;
        case i_hsize:
            if (sscanf(argstr, "%lf", &args->headersize)<1)  {
                USAGE("--header-size argument:%s was wrong. \nExample: --header-size=12.0\n", argstr);
            }
            break;
        case i_notebk:
            args->notebook = chk_onoff(argstr, usage); break;
        case i_datefmt:
            if ((argstr==NULL)||(argstr[0]=='\0')){
                args->date_format = DATE_FORMAT;
            } else {
                args->date_format = argstr;
            }
            break;
        case i_headtxt:
            if ((argstr==NULL)||(argstr[0]=='\0')){
                args->headertext = NULL;
            } else {
                args->headertext = argstr;
            }
            break;
        case i_header:
            args->noheader = !chk_onoff(argstr, usage); break;
        case i_fold_a:
            args->fold_arrow = chk_onoff(argstr, usage); break;
        case i_time_s:
            args->current_t = chk_sw(argstr, "cur", "mod", usage); break;
        case i_border:
            args->border = chk_onoff(argstr, usage); break;
        case i_punch:
            args->punchmark = chk_onoff(argstr, usage); break;
        case i_number:
            args->numbering = chk_onoff(argstr, usage); break;
        case i_side:
            args->twosides = chk_sw(argstr, "2", "1", usage); break;
        case i_unit:
            args->inch = chk_sw(argstr, "inch", "mm", usage); break;
        case i_orient:
            args->portrait = chk_sw(argstr, "p", "l", usage); break;
        case i_hslant:
            chk_slant(&args->hfont_slant, argstr, usage); break;
        case i_hweigbt:
            chk_weight(&args->hfont_weight, argstr, usage); break;
        } // switch (lindex)
    } else {
        // short option
        switch (short_index) {
        case '1':
            args->twosides=0; break;
        case '2':
            args->twosides=1; break;
        case 'b':
            args->border = 1; break;
        case 'B':
            if (sscanf(argstr, "%lf", &args->betweenline)<1) {
                USAGE("-B:(space between lines) argument:%s was wrong.\nExample: -B 1.5\n", argstr);
            }
            break;
        case 'd':
            args->duplex=1; break;
        case 'f':
            read_config(argstr);
            break;
        case 'F':
            args->fontname = argstr; break;	
        case 'h':
            help(); break;
        case 'H':
            args->noheader=1; break;
        case 'l':
            args->portrait=0; break;	
        case 'm':
            args->punchmark=1; break;
        case 'n':
            args->numbering=1; break;
        case 'o':
            args->outfile = argstr; args->one_output=1; break;
        case 'p':
            args->portrait=1; break;
        case 'P':
            args->paper = argstr; break;
        case 's':
            args->duplex=0; break;
        case 'S':
            if (sscanf(argstr, "%lf", &args->fontsize)<1) {
                USAGE("-S:(font size in pt.) argument:\"%s\" was wrong.\nExample: -S 12.0\n", argstr);
            }
            break;
        case 't':
            if (sscanf(argstr, "%d", &(args->tab))<1) {
                USAGE("-t:(tab width) argument:\"%s\" was wrong.\nExample: -t 8\n", argstr);
            }
            break;
        case 'V':
            version(); break;
        default:
            // error
            usage(NULL);
        } // switch(opt)
    }
}

void chk_slant(int *value, char *str, usage_func_t usage){
    if (strncmp(str, "normal", 8)==0)  { *value=CAIRO_FONT_SLANT_NORMAL;  return; }
    if (strncmp(str, "italic", 8)==0)  { *value=CAIRO_FONT_SLANT_ITALIC;  return; }
    if (strncmp(str, "oblique", 8)==0) { *value=CAIRO_FONT_SLANT_OBLIQUE; return; }
    USAGE("header-slant must be \"normal\", \"italic\", \"olique\",\nbut %s\n", str);
}

void chk_weight(int *value, char *str, usage_func_t usage){
    if (strncmp(str, "normal", 8)==0) { *value=CAIRO_FONT_WEIGHT_NORMAL; return; }
    if (strncmp(str, "bold", 8)==0)   { *value=CAIRO_FONT_WEIGHT_BOLD;   return; }
    USAGE("header-weight must be \"normal\" or \"bold\",\nbut %s\n", str);
}

// check arguments & return true(1)/false(0)
int chk_sw(char *str, char *positive, char *negative, usage_func_t usage) {
    if (str==NULL) { return 1; }
    else if (strncmp(str, positive, 8)==0) { return 1; }
    else if (strncmp(str, negative, 8)==0) { return 0; }
    else {
        USAGE("argument must be \"%s\" or \"%s\"\n", positive, negative);
        return -1;
    }
}

int chk_onoff(char *str, usage_func_t usage){
    return chk_sw(str, "on", "off", usage);
}

// get full path of "~/.utpdfrc" 
char *getconfpath(){
    static char path[256];
    snprintf(path, 256, "%s/%s", getenv("HOME"), CONFNAME);
    return path;
}

// str -> key, value
int parse_conf(char *str, char *key, char *value){
    int s=0, k=0, v=0;

    key[0]='\0';
    value[0]='\0';

    // comment line?
    if (str[0]=='#'){
        return 0;
    }
    // fetch keyword
    while ((str[s]!=':')&&(str[s]!='\0')&&(str[s]!='\n')){
        key[k++]=str[s++];
    }
    if ((str[s]=='\0')||(str[s]=='\n')){
        // error
        key[0]='\0';
        return 0;
    }
    key[k]='\0';
    s++;
    // skip space&tab
    while (((str[s]==' ')||(str[s]=='\t'))&&((str[s]!='\0')&&(str[s]!='\n'))){
        s++;
    }
    if ((str[s]=='\0')||(str[s]=='\n')){
        // no value
        return 1;
    }
    // fetch value
    while ((str[s]!='\0')&&(str[s]!='\n')){
        value[v++] = str[s++];
    }
    value[v]='\0';
    // remove last space&tab
    v--;
    while ((value[v]==' ')||(value[v]=='\t')){
        value[v--] = '\0';
    }
    return 2;
}

void getargs(int argc, char **argv){
    // for getopt_long()
    int opt, long_index;

    // fetch from config file
    read_config(getconfpath());
    
    // fetch from command line
    while ((opt = getopt_long
            (argc, argv, "12bB:df:F:hlmno:pP:sS:t:V", long_options, &long_index)) != -1){
        parser(opt, long_index, optarg, (usage_func_t )usage);
    }

    // input files are exist?
    if (optind >= argc) {
	usage("No file specified\n");
    }
    // twoside default setting
    if (args->twosides == -1) {
	args->twosides = (!args->portrait);
    }
    // font size
    if (args->fontsize == 0.0){
	if (!args->twosides) {
	    args->fontsize = FONTSIZE;
	} else {
	    args->fontsize = FONTSIZE_TWOSIDES;
	}
    }

    if (args->headersize == 0.0){
	if (! args->twosides) {
	    args->hfont_large = HFONT_LARGE;
	} else {
	    args->hfont_large = HFONT_TWOSIDE_LARGE;
	}
    } else {
	args->hfont_large = args->headersize;
    }
    args->hfont_medium = args->hfont_large*HFONT_M_RATE;
    args->header_height = args->hfont_large;

    // font name
    if (args->fontname == NULL) {
	args->fontname=DEFAULT_FONT;
    }

    if (args->headerfont == NULL) {
	args->headerfont = HEADER_FONT;
    }
    
    // paper
    {
	int p;
	if (args->paper != NULL){
	    for (p=0; p<PAPERS_END; p++){
		if (strncmp(args->paper, paper_sizes[p].pname, PNAME_SIZE)==0){
		    if (args->portrait){
			args->pwidth  = paper_sizes[p].w;
			args->pheight = paper_sizes[p].h;
		    } else {
			args->pwidth  = paper_sizes[p].h;
			args->pheight = paper_sizes[p].w;
		    }
		    break; 
		}
	    }
	    if (p>=PAPERS_END) USAGE("Unknown paper:%s\n", args->paper);
	} else {
	    // default
	    if (args->portrait){
		args->pwidth  = paper_sizes[PNAME_DEFAULT].w;
		args->pheight = paper_sizes[PNAME_DEFAULT].h;
	    } else {
		args->pwidth  = paper_sizes[PNAME_DEFAULT].h;
		args->pheight = paper_sizes[PNAME_DEFAULT].w;
	    }
	}
    } // end of paper

    // margins
    if (args->inch) {
	// inch -> point
	args->binding *= 72; args->outer   *= 72;
	args->ptop    *= 72; args->pbottom *= 72;
	args->divide  *= 72;
    } else {
	// mm -> point
	args->binding *= 2.8346; args->outer   *= 2.8346;
	args->ptop    *= 2.8346; args->pbottom *= 2.8346;
	args->divide  *= 2.8346;
    }
    if (args->binding == 0) { args->binding = BINDING; };
    if (args->outer   == 0) { args->outer   = OUTER;   };
    if (args->ptop    == 0) { args->ptop    = PTOP;    };
    if (args->pbottom == 0) { args->pbottom = PBOTTOM; };
    if (args->divide  == 0) { args->divide  = PBOTTOM; };
    
    if (args->binding_dir == NULL){ // default
	args->longedge = args->portrait; // shortedge = landscape(!args->portrait)
    } else {
	switch (args->binding_dir[0]){
	case 'l': // long edge
	    args->longedge = 1;
	    break;
	case 's': // short edge
	    args->longedge = 0;
	    break;
	case 'n': // no binding edge
	    args->longedge = 1;
	    args->binding = args->outer;
	    break;
	default:
	    USAGE("--binding must be \'l/s/n\', but \'%s\'\n", args->binding_dir);
	}
    }
    // end of margins
}

// end of args.c
