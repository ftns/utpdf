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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <string.h>

#include "utpdf.h"
#include "paper.h"
#include "usage.h"
#include "args.h"

#define ARGC 32

char **cmd2vec(char *str){
    static char *vec[ARGC];
    int cp=0, vp=0, prevcp=0;

    int len=strnlen(str, S_LEN);
    char *cmd=(char *)malloc(len);
    strncpy(cmd, str, S_LEN);
    
    while(cmd[cp]!='\0' && cp < len){
        if ((cmd[cp]==' ')||(cmd[cp]=='\t')){
            vec[vp++]=&cmd[prevcp];
            cmd[cp]='\0';
            cp++;
            while ((cmd[cp]==' '||cmd[cp]=='\t')&&(cp<len)) cp++;
            prevcp=cp;
        } else {
            cp++;
        }
    }
    if (prevcp<cp){
        vec[vp++]=&cmd[prevcp];
    } else {
        vp++;
    }
    vec[vp]=NULL;
    return (char **)vec;
}

void kick_pager(char *cmd) {
    char **v=cmd2vec(cmd);
    execvp(v[0], v);
}

void help(){
    int status, pid;
    int p2c_pipe[2];
    char *pager=getenv(PAGER_ENV);

    if ((pager==NULL)||(pager[0]=='\0')) {
        help_message(STDERR_FILENO);
    } else {
        if (pipe(p2c_pipe)!=0){
            perror("pipe");
            exit (1);
        }
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(1);
        }
        //
        // pipe settings:
        // parent write -> p2c_pipe[1] -> p2c_pipe[0] -> child's stdin
        //
        if (pid==0) {
            // child process            
            close(0);
            dup(p2c_pipe[0]);   // stdin(0)->p2c_pipe[0]
            close(p2c_pipe[1]); // p2c_pipe[1] is for parent
            
            kick_pager(pager);

            if ((errno==EPERM)||(errno==ENOENT)) {
                // EPERM (1): Operation not permitted.
                // ENOENT(2): No such file or directory.
                help_message(STDERR_FILENO);
                exit(1);
            } else {
                perror("execvp()");
                exit(1);
            }                
        } else {
            // parent process
            close (p2c_pipe[0]); //   p2c_pipe[0] is for child

            help_message(p2c_pipe[1]);
            if (wait(&status)<0){
                perror("wait()");
                exit(1);
            }
            exit(1);
        }
    }
}

char *get_confname(){
    if (makepdf){
        return PDF_CONF_FILE;
    } else {
        return PS_CONF_FILE;
    }
}

void help_message(int fd){
    FILE *f=fdopen(fd, "w");
    
    fprintf(f, "Usage: %s [options] <utf8_textfile> ...\n", prog_name);
    fprintf(f, "options:\n");
    fprintf(f, "  basic settings:\n");
    fprintf(f, "    -b, --border[=on/off] draw border (default: off)\n");
    fprintf(f, "    -m, --punch[=on/off]  show punch mark (default: on)\n");
    fprintf(f, "    -n, --number[=on/off] show line number (default: off)\n");
    fprintf(f, "    -t <#>, --tab=<#>     tab width (default: %d)\n", TAB);
    fprintf(f, "    --timestamp=mod/cur   timestamp: file modified time/current time\n");
    fprintf(f, "                        (file default: modified time/stdin: current time only)\n");
    fprintf(f, "    --notebook[=on/off]   show baselines like notebook\n");
    fprintf(f, "    --fold-arrow[=on/off] show the arrows indicate folded line (default: on)\n");
    fprintf(f, "\n");
    
    fprintf(f, "  sheet:\n");
    fprintf(f, "    -P, --paper=a3/a4/a5/b4/b5/b6/letter/legal\n");
    fprintf(f, "                            sheet size (default: %s)\n", paper_default());
    fprintf(f, "    -d, --duplex[=on]       duplex printing(default)\n");
    fprintf(f, "    -s, --duplex=off        simplex printing\n");
    fprintf(f, "    -l, --orientation=l     sheet orientation is landscape\n");
    fprintf(f, "    -p, --orientation=p     sheet orientation is portrait(default)\n");
    fprintf(f, "    -1, --col=1             one column per sheet (portrait default)\n");
    fprintf(f, "    -2, --col=2             two colimns per sheet (landscape default) \n");
    fprintf(f, "    --binded-edge=long/short/none\n");    
    fprintf(f, "                            bind long edge/short edge/none\n");
    fprintf(f, "                            (portrait default: long/landscape default: short)\n");
    if (!makepdf) {
    fprintf(f, "    --force-duplex[=on/off] force to duplex printing (default: off)\n");
    }
    fprintf(f, "\n");    
    fprintf(f, "  misc:\n");
    fprintf(f, "    -o <output_file>    output file\n");
    fprintf(f, "    -f <config_file>    optional config file\n");
    fprintf(f, "    -c <case_name>      load $HOME/%s-<case_name> as config file\n", get_confname());
    fprintf(f, "    -h, --help          show this message\n");
    fprintf(f, "    -V, --version       show version\n");
    fprintf(f, "    --inch, --unit=inch length unit is inch\n");
    fprintf(f, "    --mm,   --unit=mm   length unit is mm (defalt)\n");
    fprintf(f, "\n");

    fprintf(f, "  body:\n");
    fprintf(f, "    -F <fontname>, --body-font=<fontname>\n");
    fprintf(f, "                        body font (default: %s)\n", DEFAULT_FONT);
    fprintf(f, "    -S <fontsize>, --body-size=<fontsize>\n");
    fprintf(f, "                        font size in pt.\n");
    fprintf(f, "                        (default: oneside %1.1fpt./twoside %1.1fpt.)\n",FONTSIZE, FONTSIZE_TWOCOLS);
    fprintf(f, "    --body-weight=light/normal/bold/100-1000\n");
    fprintf(f, "                        body font weight (default: normal)\n");
    fprintf(f, "                        light: 300, normal: 400, bold: 700\n");
    fprintf(f, "    --body-slant=normal/italic/oblique\n");
    fprintf(f, "                        body font slant (default: normal)\n");
    fprintf(f, "    --body-spacing=<point>\n");
    fprintf(f, "                        space between lines (default: %3.2fpt)\n", BETWEEN_L);
    fprintf(f, "\n");

    fprintf(f, "  header:\n");
    fprintf(f, "    --header[=on/off]        header on/off (default: on)\n");
    fprintf(f, "    --header-text[=<text>]   center text of header (default: filename)\n");
    fprintf(f, "    --header-font=<fontname> font of center text (default: %s)\n", HEADER_FONT);
    fprintf(f, "    --header-size=<fontsize> font size of header center text\n");
    fprintf(f, "                             (default: one column %1.1fpt./two columns %1.1fpt.)\n", HFONT_LARGE, HFONT_TWOCOLS_LARGE);
    fprintf(f, "    --header-weight=light/normal/bold/100-1000\n");
    fprintf(f, "                             font weight of header center text (default: bold)\n");
    fprintf(f, "    --header-slant=normal/italic/oblique\n");
    fprintf(f, "                             font slant of header center text (default: normal)\n");
    fprintf(f, "    --header-side-size=<fontsize>\n");    
    fprintf(f, "                             font size of side part -- date & page\n");
    fprintf(f, "                             (default: %3.2f/%3.2f = <header-size> * %3.2f)\n",
            HFONT_LARGE*HFONT_M_RATE, HFONT_TWOCOLS_LARGE*HFONT_M_RATE, HFONT_M_RATE);
    fprintf(f, "    --header-side-slant=normal/italic/oblique\n");
    fprintf(f, "                             side font slant\n");
    fprintf(f, "                             (default: same as center)\n");
    fprintf(f, "    --header-side-weight=light/normal/bold/100-1000\n");
    fprintf(f, "                             side font weight (default: same as center)\n");
    fprintf(f, "    --date-format[=<format>] date format in strftime(3)\n");
    fprintf(f, "                             (default: %s)\n", DATE_FORMAT);
    fprintf(f, "\n");
    fprintf(f, "  watermark:\n");
    fprintf(f, "    --watermark-text=<text>      text of watermark\n");
    fprintf(f, "    --watermark-font=<fontname>  watermark font name (default: %s)\n", WATERMARK_FONT);
    fprintf(f, "    --watermark-slant=normal/italic/oblique\n");
    fprintf(f, "                                 watermark font slant (defaullt: normal)\n");
    fprintf(f, "    --watermark-weight=light/normal/bold/100-1000\n");
    fprintf(f, "                                 watermark font weight (default: bold)\n");
    fprintf(f, "    --watermark-color=<red>,<green>,<blue>\n");
    fprintf(f, "                                 watermark color (default: 230,230,255)\n");
    fprintf(f, "                                 each digit must be 0-255.\n");
    fprintf(f, "\n");
    fprintf(f, "  margins:\n");
    fprintf(f, "    --binding=<length>   binding margin (default: %2.1fmm/%1.1finch)\n",
            BINDING/72*25.4, BINDING/72);    
    fprintf(f, "    --left=<length>      left margin    (default: %2.1fmm/%1.1finch)\n",
            PLEFT/72*25.4, PLEFT/72);    
    fprintf(f, "    --right=<length>     right margin   (default: %2.1fmm/%1.1finch)\n",
            PRIGHT/72*25.4, PRIGHT/72);
    fprintf(f, "    --top=<length>       top margin     (default: %2.1fmm/%1.1finch)\n",
            PTOP/72*25.4, PTOP/72);    
    fprintf(f, "    --bottom=<length>    bottom margin  (default: %2.1fmm/%1.1finch)\n",
            PBOTTOM/72*25.4, PBOTTOM/72);
    fprintf(f, "    --divide=<length>    distance between two sides (default: %2.1fmm/%1.1finch)\n", DIVIDE/72*25.4, DIVIDE/72);
    fclose(f);
}


void usage(char *message){
    if (message != NULL){
        fprintf(stderr, "%s\n", message);
    }
    fprintf(stderr, "Usage: %s [-options...] <utf8_textfile> ...\n", prog_name);
    fprintf(stderr, "For more detail, %s -h\n", prog_name);
    exit(1);
}

void version(){
    fprintf(stderr, "utpdf/utps ver.%s\n", VERSION);
    fprintf(stderr, "margin-aware converter from utf-8 text to PDF/PostScript\n\n");

    fprintf(stderr, "Copyright (c) 2021 by Akihiro SHIMIZU\n\n");

    fprintf(stderr, "Licensed under the Apache License, Version 2.0 (the \"License\");\n");
    fprintf(stderr, "you may not use this file except in compliance with the License.\n");
    fprintf(stderr, "You may obtain a copy of the License at\n\n");

    fprintf(stderr, "http://www.apache.org/licenses/LICENSE-2.0\n\n");

    fprintf(stderr, "Unless required by applicable law or agreed to in writing, software\n");
    fprintf(stderr, "distributed under the License is distributed on an \"AS IS\" BASIS,\n");
    fprintf(stderr, "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n");
    fprintf(stderr, "See the License for the specific language governing permissions and\n");
    fprintf(stderr, "limitations under the License.\n");
    exit(1);
}


#ifdef SINGLE_DEBUG

int makepdf=1;
char *prog_name;

int main(int argc, char **argv){
    prog_name=argv[0];
    if (argc==1){
        help();
    }
}
#endif
// end of usage.c
