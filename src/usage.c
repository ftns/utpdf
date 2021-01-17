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

#define ARGC 32
#define MAXSTRLEN 255

char **cmd2vec(char *str){
    static char *vec[ARGC];
    int cp=0, vp=0, prevcp=0;

    int len=strnlen(str, MAXSTRLEN);
    char *cmd=(char *)malloc(len);
    strncpy(cmd, str, MAXSTRLEN);
    
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

    if (pager==NULL) {
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
            close(p2c_pipe[1]); //   p2c_pipe[1] is for parent
            
            kick_pager(pager);
        } else {
            // parent process
            close (p2c_pipe[0]); //   p2c_pipe[0] is for child

            help_message(p2c_pipe[1]);
            if (wait(&status)<0){
                perror("wait");
                exit(1);
            }
            exit(1);
        }
    }
}

void help_message(int fd){
    FILE *f=fdopen(fd, "w");
    
    fprintf(f, "Usage: %s [options] <utf8_textfile> ...\n", prog_name);
    fprintf(f, "options:\n");
    fprintf(f, "  printing:\n");
    fprintf(f, "    -b --border[=on/off] draw border\n");
    fprintf(f, "    -B <point>           space between lines (default: 1.0pt)\n");
    fprintf(f, "    -m --punch[=on/off]  show punch mark\n");
    fprintf(f, "    -n --number[=on/off] show line number\n");
    fprintf(f, "    --notebook[=on/off]  show baselines like notebook\n");
    fprintf(f, "    --fold-arrow[=on/off]\n");
    fprintf(f, "                         show arrow indicate folded line (default: on)\n");
    fprintf(f, "    -t <#>               tab width (default: %d)\n", TAB);
    fprintf(f, "    --timestamp=mod/cur  timestamp: file modified time/current time\n");
    fprintf(f, "                         (file default: modified time/stdin: current time only)\n");
    fprintf(f, "    -F <fontname>        body font (default: %s)\n", DEFAULT_FONT);
    fprintf(f, "    -S <fontsize>        font size in pt.\n");
    fprintf(f, "                         (default: oneside %1.1fpt./twoside %1.1fpt.)\n",
            FONTSIZE, FONTSIZE_TWOSIDES);
    fprintf(f, "  paper:\n");
    fprintf(f, "    -P a3/a4/a5/b4/b5/b6/letter/legal\n");
    fprintf(f, "                         paper size (default: %s)\n", paper_default());
    fprintf(f, "    -d --duplex[=on]     duplex printing(default)\n");
    fprintf(f, "    -s --duplex=off      simplex printing\n");
    fprintf(f, "    -l                   landscape\n");
    fprintf(f, "    -p                   portrait(default)\n");
    fprintf(f, "    -1 --side=1          one side per page (portrait default)\n");
    fprintf(f, "    -2 --side=2          two sides per page (landscape default) \n");
    fprintf(f, "    --binddir=l/s/n      bind long edge/short edge/none\n");
    fprintf(f, "                   (portrait default: long edge/landscape default: short edge)\n");
    fprintf(f, "  misc:\n");
    fprintf(f, "    -o <filename>        output file name\n");
    fprintf(f, "    -h --help            show this message\n");
    fprintf(f, "    -V --version         show version\n");
    fprintf(f, "    \n");
    fprintf(f, "  length unit:\n");
    fprintf(f, "    --inch               unit is inch\n");
    fprintf(f, "    --mm                 unit is mm (defalt)      \n");
    fprintf(f, "  header:\n");
    fprintf(f, "    --header[=on/off]    header on/off (default: on)\n");
    fprintf(f, "    --header-font=<fontname> header font\n");
    fprintf(f, "                         (default: sans-serif)\n");
    fprintf(f, "    --header-size=<fontsize> header font size\n");
    fprintf(f, "                         (default: oneside %1.1fpt./twoside %1.1fpt.)\n",
            HFONT_LARGE, HFONT_TWOSIDE_LARGE);
    fprintf(f, "    --header-text=<text> header center text (default: filename)\n");
    fprintf(f, "    --date-format=<format> \n");
    fprintf(f, "                         date format in strftime(3) (default: %s)\n",
            DATE_FORMAT);
    fprintf(f, "  margins:\n");
    fprintf(f, "    --binding=<length>   binding margin (default: %2.1fmm/%1.1finch)\n",
            BINDING/72*25.4, BINDING/72);    
    fprintf(f, "    --outer=<length>     outer margin   (default: %2.1fmm/%1.1finch)\n",
            OUTER/72*25.4, OUTER/72);    
    fprintf(f, "    --top=<length>       top margin     (default: %2.1fmm/%1.1finch)\n",
            PTOP/72*25.4, PTOP/72);    
    fprintf(f, "    --bottom=<length>    bottom margin  (default: %2.1fmm/%1.1finch)\n",
            PBOTTOM/72*25.4, PBOTTOM/72);
    fprintf(f, "    --divide=<length>    distance between two sides (default: %2.1fmm/%1.1finch)\n",
            DIVIDE/72*25.4, DIVIDE/72);
    fclose(f);
}


void usage(char *message){
    if (message != NULL){
        if (errno != 0){
            perror(message);
        } else {
            fprintf(stderr, "%s\n", message);
        }
    }
    fprintf(stderr, "\nUsage: %s [-12bBdFlmnstSPp...] <utf8_textfile> ...\n", prog_name);
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
int main(int argc, char **argv){
    prog_name=argv[0];

    help();
}
#endif
// end of usage.c
