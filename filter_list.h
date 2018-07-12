#ifndef FILTER_LIST_H

#define FILTER_LIST_H

#include <stdio.h>

#define UML_FILE_NAME "seq_uml.dot"
#define LINE_LENGTH 2048
#define DATE_LENGTH 32 
#define TIME_LENGTH 32 
#define LOG_LENGTH 2048 
#define PROC_NAME_1 64 
#define PROC_NAME_2 64 
#define PID_LENGTH 32 
#define MAX_LINE_CACHE 128
#define SIMILAR_DISTANCE 4
#define SUBLINE_LENGTH 64
#define INCLUDE_DISTANCE 40
#define ALL_TXT "insight.out"


int bl,wl;

extern char filter_list[500][50];
extern char *include_filter_list[500][150];
extern FILE *seq_uml_prologue();
extern int seq_uml_epilogue(FILE *fp);

int add_seq_log_entry(FILE *fp, char *date, char *log_time,
                      char *tag,
                      char *proc_name_1,
                      char *proc_name_2, int pid,
                      char *log,int dt);
int distance (const char * word1,
                     int len1,
                     const char * word2,
                     int len2);

#endif
