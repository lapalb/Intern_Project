/*
 Insight - A smart distributed log analyser which is capable of process based filtering, 
            and timestamp based filtering and give result as svg, html, or text
  Company: Cisco
  Mentored by: Ganesh Shankar(Cisco Id: gashank2)
  Intern_Name: Ashish Kumar Jha(Cisco Id: asjha2) : amiableashish55@gmail.com
*/


/*! \mainpage A briefer introduction to Insight
    \section  Introduction
      A smart distributed log analyser which is capable of process based filtering, and timestamp based filtering and give result as svg, html, or text
 
---
<BR>

\section cmd_frmt Synopsis
<BLOCKQUOTE>./insight -s subsys -t time_ref  -b n_before_time_ref -a n_after_time_ref -d Divider_Time -f html|seq|text -e black_list_file_name -i white_list_file_name trace_file.txt 2>err >insight.out </BLOCKQUOTE>

---
<BR>

\section diff_flg Description
\subsection subsys s: subsystem list
This flag is used for process based filtering. When any process is specified only those log which contain that process is parsed.
~~~~~~~~~~~~~{.sh}
./insight -s iomd,pamnlog,asic all.txt
~~~~~~~~~~~~~

---
<BR>

\subsection time  t: time_reference(time_ref): 
This flag is used to specify the base time from which logs are considered. Another two argument <I><B> Before_time </I></B> amd <I> <B>After time </I/></B>is specified
for timestamp based filtering.
~~~~~~~~~~~~~{.sh}
./insight -t 03:05:04:625 all.txt
~~~~~~~~~~~~~

---
<BR>

\subsection time_before b:n_before_time_reference 
This flag is used to specify the time in milisecod before the specified time refrence which is to be considered
~~~~~~~~~~~~~{.sh}
./insight -t 03:05:04:625 -b 1000 all.txt
~~~~~~~~~~~~~

---
<BR>

\subsection time_after a: n_after_time_ref 
This flag is used to specify the time in milisecod after the specified time refrence which is to be considered
~~~~~~~~~~~~~{.sh}
./insight -t 03:05:04:625 -a 3000 all.txt
~~~~~~~~~~~~~

---
<BR>

\subsection divider_time d: divider time
This is used to have divider in the sequence diagram. It is specified in milisecond
~~~~~~~~~~~~~{.sh}
./insight -t 03:05:04:625 -a 3000 -d 100 all.txt
~~~~~~~~~~~~~

---
<BR>

\subsection format f: format html| seq 
This is used to specify the format of output. Report can be generated in html format or sequence diagram. Remember that to genearte
sequence diagram, output need to be directed into a file called <B> insight.out</B>
~~~~~~~~~~~~~{.sh}
./insight -f seq bt_all.txt > insight.out
~~~~~~~~~~~~~

---
<BR>

\subsection whitelist_file i:whitelist file
This flag is used to specify the white list file. Whitelist file contains the
name of different process separated by newline.
~~~~~~~~~~~~~{.sh}
./insight -i w.txt all.txt
~~~~~~~~~~~~~

---
<BR>

\subsection blacklist_file e  e:blacklist file
This flag is used to specify the black list file. Blacklistfile contains the
name of different process separated by newline.
~~~~~~~~~~~~~{.sh}
./insight -e b.txt all.txt
~~~~~~~~~~~~~


---
<BR>
<BR>
<BR>
\subsection Exg Example

~~~~~~~~~~~~~{.sh}
./insight -s cmand,iomd,cmcc  -f seq -t 03:05:07:456 -b 1000 -a 2000 -d 1000 -e b.txt -i w.txt  bt_all.txt 2>err >insight.out

~~~~~~~~~~~~~
<IMG SRC="C:\Users\asjha2\Documents\html\Capture.png" style="width=100%">
<BR>
<BR>
<BR>

\section Author
Written by [Ganesh Shankar](https://www.linkedin.com/in/ganesh-h-shankar-4926aa1b) and [Ashish Jha](https://www.linkedin.com/in/ashish-jha-6bb7b3101/)

\section Reporting Bugs
Mail me at [Ashish Jha](mailto:amiableashish55@gmail.com) 

\copyright  [Cisco India](https://www.cisco.com/c/en_in/index.html)               
For Cisco Internal Use 
 */
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "filter_list.h"
#include "filter.h"

FILE * fphtml; /*!<This will open an html file for html output geneartion */

/**Defining a cache to hold last 128 log lines so that we can remove similar log message*/
struct l_cache {
    int index;
    char last_lines[MAX_LINE_CACHE][LINE_LENGTH];
}lines_cache;

/** This function insert the current line into Circular Cache.*/
void insert_line(char *line){
   int i;
   i = lines_cache.index;
   snprintf(lines_cache.last_lines[i],LINE_LENGTH,"%s",line);
   i = (i+1) % MAX_LINE_CACHE;
   lines_cache.index = i;
}

/** Check if cuurent Log is similar to last 128 Log and Last 128 Log is stored in a structure **l_cacahe** */
int is_similar(char *part_log, int len1){
   int i, d;
   for (i=0;i<lines_cache.index;i++) {
       d = distance(part_log, len1, lines_cache.last_lines[i], strlen(lines_cache.last_lines[i]));

       if (d <= SIMILAR_DISTANCE) {
           return 1;
       }
   }
   return 0;
}


/**This function set the lowerbound of the log_time to be included. 
It convert time into epoch*/
int lowBound(char * log_time, char * str, int time_delta) {
  if(str[0]=='\0')return 1;

  char log_timel[15]; //Local Log Time
  strcpy(log_timel,log_time);
  int log_timei=0;
  char *ptr=strtok(log_timel,":.");
  int i=1;
  while(ptr!=NULL)
    { 
        if(i<=3600)
           {
                log_timei+= atoi(ptr)*3600*1000/i;
                ptr=strtok(NULL,":.");
                i*=60;     
          }
        else
          { 
                log_timei+= atoi(ptr);
                ptr=strtok(NULL,":.");
          }
    } 
 
  char strl[15];
  strcpy(strl,str);
  int stri=0;
  ptr=strtok(strl,":.");
  int j=1;
  while(ptr!=NULL)
   {
      if(j<=3600)
      {
         stri+= atoi(ptr)*3600*1000/j; 
         ptr=strtok(NULL,":.");j*=60;
      }
      else
      { 
        stri+= atoi(ptr);
        ptr=strtok(NULL,":.");
      }
   }
  stri=stri - time_delta;
  if(log_timei>stri)
    return 1;
  else
  return 0;
 }

/**This function set the UpperBound of the log_time to be included. It convert time into epoch*/
int upBound(char * log_time, char * str, int time_delta) {
    if(str[0]=='\0')return 1;
    char log_timel[15];
    strcpy(log_timel,log_time);
    int log_timei=0;
    char *ptr=strtok(log_timel,":.");
    int i=1;
    while(ptr!=NULL){ 
          if(i<=3600){
                log_timei+= atoi(ptr)*3600*1000/i;
                ptr=strtok(NULL,":.");
                i*=60;     
            }
          else{ 
                log_timei+= atoi(ptr);
                ptr=strtok(NULL,":.");
            }
      } 
 
    char strl[15];
    strcpy(strl,str);
    int stri=0;
    ptr=strtok(strl,":.");
    int j=1;
    while(ptr!=NULL)
     {
        if(j<=3600){
           stri+= atoi(ptr)*3600*1000/j; 
           ptr=strtok(NULL,":.");j*=60;
        }
        else{ 
          stri+= atoi(ptr);
          ptr=strtok(NULL,":.");
        }
     }
    stri=stri + time_delta;
    if(log_timei<stri)
      return 1;
    else
    return 0;
}
     

//2018/05/10 
#define DATE_LENGTH_CHECK 10
/** This function is used for parsing ta log line and return different value accoridng to different condition matched */
int time_filter(char *line, int line_no, char *date, char *log_time, char *proc, char *proct, int *pid, char *log,char * t, int time_deltab,int time_deltaf){
    char *cp=line;
    int r, colon_count,i;
    char cpid[PID_LENGTH];
    static char prev_line[LINE_LENGTH];
    static int prev_failed,first_print;
    // 2018/03/27 20:44:56.481 {chasync_R0-0}{1}: [chasync] [16185]: UUID: 0, ra: 0, TID: 0 (note):  MASTERSHIP: CLOSE_WRITE-CLOSE standby-rp-state

    while (*cp && !isdigit(*cp)) cp++; 
    if (!cp) return -1;

    r = sscanf(line,"%s %s %s %s %s",date,log_time,proc,proct,cpid);
    if ( (r != 5) || (strlen(date) < DATE_LENGTH_CHECK)) {
      // Invalid line
     if (prev_failed && first_print) {
          fprintf(stderr,"%s %d. %s\n",prev_line,line_no,line);
          first_print = 0;
     }
     else {
          fprintf(stderr,"%d. %s\n",line_no,line);
     }
     prev_failed = 1;
     return -1;
    }

    if(!lowBound(log_time,t,time_deltab))return -90;
    if(!upBound(log_time,t,time_deltaf))return -110;
    prev_failed = 0;
    first_print = 1;
    //Sanitising the parse line
    cp = strchr(proc,':');
    if (cp) *cp = '\0';

    *pid = 0;

    cp = strchr(cpid,':');
    if (cp) *cp = '\0';

    cp = strchr(cpid,'[');
    if (cp) *cp = ' ';

    cp = strchr(cpid,']');
    if (cp) *cp = '\0';

    *pid = atoi(cpid);

    cp = line;
    cp += 24;

    /* We are here now.
     *
     * {chasync_R0-0}{1}: [chasync] [16185]: UUID: 0, ra: 0, TID: 0 (note):  MASTERSHIP: CLOSE_WRITE-CLOSE standby-rp-state 
     * 
     */

    colon_count = 0;
    while ( (*cp) && (colon_count < 6)) {
        if ( *cp == ':' ) colon_count++;
        cp++;
    }

    if ( (colon_count < 6) && !(*cp)) return -1;
    strcpy(log,cp);
    if(strstr(log,"message@")!=NULL) return 50;
    snprintf(prev_line,sizeof(prev_line),"%s",line);

    return 0;
}



/** This function is used for filtering the log based on blacklist file specified*/
int to_be_filtered(char *log_line){ 
   for(int i=0;i<(bl-1);i++){
      if(strstr(log_line,filter_list[i])){return 1;}
  }
  return 0;
}
 
/*-----------*/
#define SUBSYS_LEN 256 /*!< */
#define FORMAT_LENGTH 32
#define FILE_NAME_LENGTH 128
#define FILE_NAME_LENGTH 128
#define TAG_LENGTH 32
#define TIME_REF_LENGTH 32
int con=0; /*!< A flag to check if we are dealing with multi line log.
For example: message@
*/

/** Data structure to hold the different filter passed by Command line reference */
struct prog_options {
   char sub_sys[SUBSYS_LEN];/*!< name of subprocess that is to be included*/
   char format[FORMAT_LENGTH]; /*!<  Foramt of output. It can be html or svg*/
   char white_list_file[FILE_NAME_LENGTH];/*!< Name of whitelist file*/
   char black_list_file[FILE_NAME_LENGTH];/*!< Name of black list file*/
   char time_ref[TIME_REF_LENGTH];/*!< The time reference that is to be considered*/
   int logs_before;/*!< How much time before specified time is to be considered*/
   int logs_after;/*!<How much mili-second after the specified time is to be consdered  */
   char trace_file_name[FILE_NAME_LENGTH];/*!< Name of log File*/
   char tag[TAG_LENGTH];/*!< */
   int dt; /*!< dt, short for delta time. This parameter is used when we need divider in sequence diagram*/
   };

//char line[256] = "2018/03/27 20:44:56.481 {chasync_R0-0}{1}: [chasync] [16185]: UUID: 0, ra: 0, TID: 0 (note):  MASTERSHIP: CLOSE_WRITE-CLOSE standby-rp-state";
struct parse_filters_info g_parse_filters;
char message_log[10000]; /*!< A place holder for holding nulti line message log */

/** This function parses the entire log file */
int parse_log_file(FILE *fp,struct prog_options *opts){
    // For Parsing the individual lines
    char line[LINE_LENGTH];
    char date[DATE_LENGTH],log_time[TIME_LENGTH];
    char log[LOG_LENGTH];
    char proc_name_1[PROC_NAME_1];
    char proc_name_2[PROC_NAME_2];
    int pid, parse_fails=0;
    int line_no=1;
    char cache_check[SUBLINE_LENGTH];
    char *cp;

    // Filtering Variables of insight
    int success_count=0;
    int filter_blacklist_count=0;
    int is_similar_count=0;
    int time_filter_forward_count=0;
    int time_filter_back_count=0;
    int subsys_filter_count=0;
     
    //for(int i=0;i<bl;i++)printf("%s",filter_list[i]);
    //for(int i=0;i<wl;i++)printf("wl:%s\n",include_filter[i]);
    while ( fgets(line,sizeof(line),fp) ) {
        int ret=time_filter(line, line_no, date, log_time, proc_name_1, 
                         proc_name_2, &pid, log, opts->time_ref , opts -> logs_before,opts->logs_after) ; 
       //If global variable con is 1, we are parsing message @ and we will parse it until we get a correct line i.e. ret=1
        if(con==1&& ret==-1){
          strcat(message_log,line);
          continue;
        }
        //Finally We get properly Formatted Line so we set global variable con=0
        if(ret!=-1 && con==1){
         	 con=0;
          	int count = 0;
          	char m[10000];
          	for (int i = 0; message_log[i]; i++){ 
          		if ( message_log[i]!='\n' )
           			m[count++] = message_log[i]; 
          		m[count] = '\0';
          	}	
    
     	  	fprintf(stdout," %s %s L%d %s %s %d :: %s\n",date,log_time,line_no,proc_name_1,proc_name_2,pid,m);
         	 memset(message_log, 0, strlen(message_log));
          	continue;
        }
      
        if(ret==-1){
                      parse_fails++;line_no++;
                      continue;
        }
       
   if ( to_be_filtered(proc_name_1) ) {
          line_no++;
          filter_blacklist_count++;
          fprintf(stderr,"Ignored due to blacklist: %s",line);
          continue;
        }
    if ( ! filter_include_subsys(proc_name_1, &g_parse_filters) ) {
                line_no++;
                subsys_filter_count++;
                fprintf(stderr,"IGNORED due to subsys filter: %s\n",line);
                continue;
         }
         

        if(ret==-110){line_no++;time_filter_forward_count++ ;fprintf(stderr,"Ignored due to forward time filter %s\n",line);continue; };
        if(ret==-90){line_no++;time_filter_back_count++ ;fprintf(stderr,"Ignored due to backward time filter %s\n",line);continue; };
        

         /* Check Lavenstein's distance to prune out similar logs */
         snprintf(cache_check,sizeof(cache_check),"%s",log);
        if ( is_similar(cache_check, strlen(cache_check)) ) {
             insert_line(cache_check);
             fprintf(stderr,"%d. Similar!! Ignored. Previous substring: %s Line: %s\n",line_no,line,cache_check);
             line_no++;
             is_similar_count++;
             continue;
         }

         //First Occurence of Message@
        if(ret==50){
          strcat(message_log,log);
          con=1;
          continue;
        }

         line_no++;
         insert_line(cache_check);
         success_count++;
         fprintf(stdout," %s %s L%d %s %s %d :: %s",date,log_time,line_no,proc_name_1,proc_name_2,pid,log);
  
          if(strcmp(opts->format, "html")==0)
          {
            
            if(!fphtml){printf("Failed to Open");exit(0);}
            else{
              fputs("<tr>",fphtml);
              fputs("<td>",fphtml);
              fputs(date,fphtml);
              fputs("</td>",fphtml);

              fputs("<td>",fphtml);
                 fputs(log_time,fphtml);
              fputs("</td>",fphtml);


              fputs("<td>",fphtml);
              char l[10];
              sprintf(l, "%d", line_no);
              fputs(l,fphtml);
              fputs("</td>",fphtml);


              fputs("<td>",fphtml);
                 fputs(proc_name_1,fphtml);
              fputs("</td>",fphtml);

              fputs("<td>",fphtml);
                 fputs(proc_name_2,fphtml);
              fputs("</td>",fphtml);


              fputs("<td class='dan'>",fphtml);
              fputs(log,fphtml);
              fputs("</td>",fphtml);
            
              fputs("</tr>",fphtml);
            }
          }
        
    }
    
    fprintf(stderr,"=====================================Insight====================================\n");
    fprintf(stderr,"\t\t         Unsuccessful Parsed : %20d\n",parse_fails);
    fprintf(stderr,"\t\t Filteres due to Time Forward: %20d\n",time_filter_forward_count);
    fprintf(stderr,"\t\t Filteres due to Time Backard: %20d\n",time_filter_back_count);
    fprintf(stderr,"\t\tFilteres due to Subsys Filter: %20d\n",subsys_filter_count);
    fprintf(stderr,"\t\t    Filtered due to BlackList: %20d\n",filter_blacklist_count); 
    fprintf(stderr,"\t\t   Filtered due to Similarity: %20d\n",is_similar_count);    
    fprintf(stderr,"\t\t            Successful Parsed: %20d\n",success_count);   
    fprintf(stderr,"\t\t                 Total Parsed: %20d\n",line_no);
    fprintf(stderr,"=====================================Insight====================================\n");
   
}

/** This function is used to set the default option*/
void set_default_options(struct prog_options *opts, int argc){

    strcpy(opts -> sub_sys,"all");
    strcpy(opts -> format,"text");
    opts -> logs_before = 0;
    opts -> logs_after = 0;
    opts -> white_list_file[0] = '\0';
    opts -> black_list_file[0] = '\0';
    opts -> time_ref[0] = '\0';
    opts -> tag[0] = '\0';
    opts -> trace_file_name[0] = '\0';
    opts-> dt=1000;
}
/** This function print the valueof different flag specified to stderr for debugging */
void print_options(struct prog_options *opts){
    fprintf(stderr,"\n\nSub_sys: %s\nFormat: %s\nlogs_before: %d\nlogs_after:%d\nWhite list: %s\nBlack list: %s\ntime_ref: %s\ntrace_file_name: %s\nTag: %s\n Divider_Time: %d\n\n",
            opts -> sub_sys,
            opts -> format,
            opts -> logs_before, 
            opts -> logs_after,
            opts -> white_list_file, 
            opts -> black_list_file, 
            opts -> time_ref, 
            opts -> trace_file_name,
            opts -> tag,
            opts -> dt
            
          );
}

/** This function print the help command*/
void help(char *prog_name){
    fprintf(stdout,"=====================================Insight====================================\n");
    fprintf(stdout,"A smart distributed log analyser which is capable of process based filtering,and timestamp based filtering \nand give result as svg, html, or text \n");
    fprintf(stdout,"=====================================NOTE====================================\n");
    fprintf(stdout,"To get sequence diagram, You MUST redirect the output to \" insight.out\" file.\n");
    fprintf(stdout,"In blacklist or whitelist file, process should be without quotes and be on separate line\n");
    fprintf(stdout,"=====================================Insight====================================\n");
    fprintf(stdout,"\n\nTo invoke type: \n");
    fprintf(stdout,"%s -s subsys -t time_ref  -b n_before_time_ref -a n_after_time_ref -d Divider_Time -f html|seq|text -e black_list_file_name -i white_list_file_name trace_file.txt 2>err >insight.out\n\n", prog_name);
     fprintf(stdout,"Options:\n"
                  "     -s comma sperated process name  ->  subsystem list. When any process is specified only those log which contain that process is parsed.\n"
                  "     -t base time                    -> the base time from which logs are considered..\n"
                  "     -b time before base time in msec-> the time in milisecod before the specified time refrence which is to be considered\n"
                  "     -a time after base time in msec -> time in milisecod after the specified time refrence which is to be considered\n"
                  "     -d divider time in mesec        -> for divider in the sequence diagram. It is specified in milisecond\n"
                  "     -f format in msec               ->  format :(html| seq)This is used to specify the format of output\n"
                  "     -i white_list_file_name         ->Whitelist file.This flag is used to specify the white list file. \n"
                  "     -b black_list_file_name         -> Whitelist file.This flag is used to specify the black list file. \n\n");

   }

/* "s:t:m:b:a:f:e:i:g:h" */

#define PROG_OPTIONS "s:t:m:b:a:f:e:i:g:h:d:"
/** This function craete the html file */
void html(){
    fphtml=fopen("result.html","w");;
    char buf[2000]="<html><head><title> Result of Insight</title><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'> <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script> <script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script> <style>th{text-align:center} h1{color: blue;text-align: center} .dan{word-break: break-all;}td{text-align:justify}</style> </head><body> <div class='container-fluid'><h1><b>Result of Insight</b></h1> <table class='table table-bordered table-striped table-responsive'> <thead><tr><th>Date</th><th>Log Time</th><th>Line Number</th><th>Process Name 1</th><th> Process Name 2</th><th> Log Message</th></tr></thead>";
    fputs(buf,fphtml);
}
/** This function generate the sequnece diagram*/
void seq( struct prog_options *opts){
    system("make seq_diag >/dev/null");
    char p[15];
    sprintf(p,"./seq_diag %d",opts->dt);
    system(p);
    system("java -jar plantuml.jar -tsvg seq_uml.dot");
}

/** Internal function used by progeam to parse the command line argument*/
int parse_options(int argc, char *argv[], struct prog_options *opts){
  int c;
  opterr = 0;
  set_default_options(opts, argc);
  if (argc == 1) {
     help(argv[0]);
     exit(0);
  }

  while ((c = getopt (argc, argv, PROG_OPTIONS)) != -1) {
    switch (c) {
      
      case 'd':
        opts -> dt = atoi(optarg);           
        dt=opts -> dt;
        //fprintf(stderr,"opts -> dt: %s\n",dt);
        break;

      case 's':
        strcpy(opts -> sub_sys,optarg);
        set_subsys_filters(opts -> sub_sys, &g_parse_filters);
        fprintf(stderr,"opts -> sub_sys: %s\n",opts -> sub_sys);
        break;

      case 't':
        strcpy(opts -> time_ref,optarg);
        break;

      case 'f':
        strcpy(opts -> format,optarg);
        if(strcmp(opts -> format,"html")==0)html();
        break;

      case 'b':
        opts -> logs_before = atoi(optarg);
        break;

      case 'a':
        opts -> logs_after = atoi(optarg);
        break;

      case 'i':
        strcpy(opts -> white_list_file,optarg);
        char linei[256];
        FILE * fpi=fopen(opts->white_list_file,"r");
        if(!fpi) 
        {
          printf("White List File Specified doesn't exist");
          exit(0);
        }  
        int i=0;
        while(!feof(fpi)){
          if(fgets(linei,150,fpi)){
              char *p;
              p = strchr(linei, '\n');
              if (p != NULL)
                *p = '\0';
              strcpy(include_filter_list[i],linei); 
              i++;
          }
         wl=i-1;
        }
       break;
      case 'e':
        strcpy(opts -> black_list_file,optarg);
        char linee[256];
        FILE * fpe=fopen(opts->black_list_file,"r");
        if(!fpe) {
          printf("Black List File Specified doesn't exist");
          exit(0);
        } 

        int e=0;
        while(!feof(fpe)){
          if(fgets(linee,50,fpe)!=NULL){
            char *p;
            p = strchr(linee, '\n');
            if (p != NULL)
              *p = '\0';
            strcpy(filter_list[e], linee); 
            e++;
          }
         bl=e-1;
        }   
        break;

      case 'g':
        strcpy(opts -> tag,optarg);
        break;

      case 'h':
        help(argv[0]);
        exit(0);
      
      default:
        help(argv[0]);
        exit(0);
    }
 }

  strcpy(opts -> trace_file_name,argv[argc-1]);
  print_options(opts);
  return 0;

}

int main(int argc, char *argv[]){
    //setvbuf ( stdout ,_IONBF);
    struct prog_options opts;
    FILE *fp;
    parse_options(argc, argv, &opts);
    fp = fopen(opts.trace_file_name, "r");
    if (!fp) {
        fprintf(stderr,"File %s does not exist\n",argv[1]);
        exit(1);
    } 
    parse_log_file(fp, &opts);
    fclose(fp);fflush(stdout);
    if(strcmp(opts.format,"seq")==0)seq(&opts);
   }
